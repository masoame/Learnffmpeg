#pragma once
#include"common.hpp"

struct Clannel_AVFrame
{
	uint8_t* data[AV_NUM_DATA_POINTERS];
	int linesize[AV_NUM_DATA_POINTERS];
};

class LearnVideo
{
public:
	enum RESULT
	{
		SUCCESS, UNKONW_ERROR, ARGS_ERROR,
		ALLOC_ERROR, OPEN_ERROR
	};

	LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context";  };
	~LearnVideo() {};
	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	RESULT close();
	
	//初始化所有参数
	RESULT init();
	//初始化音视频解码器
	RESULT init_decode();
	//初始化音频转化器
	RESULT init_swr(const AVChannelLayout* out_ch_layout,const enum AVSampleFormat out_sample_fmt,const int out_sample_rate);
	//帧格式转化
	RESULT init_sws(const AVFrame* avf,const AVPixelFormat dstFormat,const int dstW = 0,const int dstH = 0);

	//开始转化图像帧
	RESULT start_sws(const AVFrame* avf);
	//开始音视频解码
	RESULT start_video_decode(const std::function<bool(AVFrame*)>& video_action = nullptr, const std::function<bool(AVFrame*)>& audio_action = nullptr);
	//音视频编码
	RESULT init_encode(const enum AVCodecID encodeid, AVFrame* frame);
	//
	RESULT start_video_encode(const AVFrame* frame);

	const AVFormatContext *  get_avfctx() { return avfctx; }
private:

	AutoAVFormatContextPtr avfctx;
	AutoAVCodecContextPtr decode_video_ctx, decode_audio_ctx, encode_video_ctx, encode_audio_ctx;
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

	//用于引用avfctx的音频与视频流的下标
	int AVStreamIndex[8];

	//音频缓存队列以及音频锁
	std::queue<AutoAVFramePtr> audio_queue;
	std::mutex audio_queue_mtx;

	//视频缓存队列以及视频锁
	std::queue<AutoAVFramePtr> video_queue;
	std::mutex video_queue_mtx;
	
};