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
	
	//��ʼ�����в���
	RESULT init();
	//��ʼ������Ƶ������
	RESULT init_decode();
	//��ʼ����Ƶת����
	RESULT init_swr(const AVChannelLayout* out_ch_layout,const enum AVSampleFormat out_sample_fmt,const int out_sample_rate);
	//֡��ʽת��
	RESULT init_sws(const AVFrame* avf,const AVPixelFormat dstFormat,const int dstW = 0,const int dstH = 0);

	//��ʼת��ͼ��֡
	RESULT start_sws(const AVFrame* avf);
	//��ʼ����Ƶ����
	RESULT start_video_decode(const std::function<bool(AVFrame*)>& video_action = nullptr, const std::function<bool(AVFrame*)>& audio_action = nullptr);
	//����Ƶ����
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

	//��������avfctx����Ƶ����Ƶ�����±�
	int AVStreamIndex[8];

	//��Ƶ��������Լ���Ƶ��
	std::queue<AutoAVFramePtr> audio_queue;
	std::mutex audio_queue_mtx;

	//��Ƶ��������Լ���Ƶ��
	std::queue<AutoAVFramePtr> video_queue;
	std::mutex video_queue_mtx;
	
};