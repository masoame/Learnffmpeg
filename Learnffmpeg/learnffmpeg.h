#pragma once
#include"common.h"

struct Channel_Planes
{
	explicit Channel_Planes(std::initializer_list<std::pair<uint8_t*, int>> list)
	{
		int count = 0;
		for (auto& ptr : list)
		{
			_data[count].reset(ptr.first);
			linesize[count] = ptr.second;
			if (++count == AV_NUM_DATA_POINTERS) break;
		}
	}
	constexpr uint8_t* const* data() { return (uint8_t* const*)_data; }
	//通道数据指针
	std::unique_ptr<uint8_t[]> _data[AV_NUM_DATA_POINTERS]{ nullptr };
	//每个通道大小数组
	int linesize[AV_NUM_DATA_POINTERS]{ 0 };
};

class LearnVideo
{
public:
	LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context";  };
	~LearnVideo() {};
	bool open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	bool close();
	
	//初始化所有参数
	bool init();
	//初始化音视频解码器
	bool init_decode();
	//初始化音频转化器
	bool init_swr(const AVChannelLayout* out_ch_layout,const enum AVSampleFormat out_sample_fmt,const int out_sample_rate);
	//帧格式转化
	bool init_sws(const AVFrame* avf,const AVPixelFormat dstFormat,const int dstW = 0,const int dstH = 0);

	//开始转化
	void start_sws(const AVFrame* avf, Channel_Planes* dstch);


	//开始音视频解码
	bool start_video_decode(const std::function<bool(AVFrame*)>& video_action = nullptr, const std::function<bool(AVFrame*)>& audio_action = nullptr);
	//音视频编码
	bool init_encode(const enum AVCodecID encodeid, AVFrame* frame);
	bool start_video_encode(const AVFrame* frame);
	//

	//
	const AVFormatContext *  get_avfctx() { return avfctx; }
private:

	AutoAVFormatContextPtr avfctx;
	AutoAVCodecContextPtr decode_video_ctx, decode_audio_ctx, encode_video_ctx,encode_audio_ctx;
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

	//用于引用avfctx的音频与视频流
	int AVStreamIndex[8];
};