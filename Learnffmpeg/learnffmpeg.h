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
	//ͨ������ָ��
	std::unique_ptr<uint8_t[]> _data[AV_NUM_DATA_POINTERS]{ nullptr };
	//ÿ��ͨ����С����
	int linesize[AV_NUM_DATA_POINTERS]{ 0 };
};

class LearnVideo
{
public:
	LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context";  };
	~LearnVideo() {};
	bool open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	bool close();
	
	//��ʼ�����в���
	bool init();
	//��ʼ������Ƶ������
	bool init_decode();
	//��ʼ����Ƶת����
	bool init_swr(const AVChannelLayout* out_ch_layout,const enum AVSampleFormat out_sample_fmt,const int out_sample_rate);
	//֡��ʽת��
	bool init_sws(const AVFrame* avf,const AVPixelFormat dstFormat,const int dstW = 0,const int dstH = 0);

	//��ʼת��
	void start_sws(const AVFrame* avf, Channel_Planes* dstch);


	//��ʼ����Ƶ����
	bool start_video_decode(const std::function<bool(AVFrame*)>& video_action = nullptr, const std::function<bool(AVFrame*)>& audio_action = nullptr);
	//����Ƶ����
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

	//��������avfctx����Ƶ����Ƶ��
	int AVStreamIndex[8];
};