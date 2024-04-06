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

	using backcall_avframe = bool(*)(AutoAVFramePtr&&);
	LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context"; };
	~LearnVideo() {};
	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	RESULT close();

	//��ʼ�����в���
	RESULT init();
	//��ʼ������Ƶ������
	RESULT init_decode();
	//��ʼ����Ƶת����
	RESULT init_swr(const AVChannelLayout* out_ch_layout, const enum AVSampleFormat out_sample_fmt, const int out_sample_rate);
	//֡��ʽת��
	RESULT init_sws(const AVFrame* avf, const AVPixelFormat dstFormat, const int dstW = 0, const int dstH = 0);

	//��ʼת��ͼ��֡
	RESULT start_sws(const AVFrame* avf);
	//��ʼ����Ƶ����
	RESULT start_video_decode();
	//����Ƶ����
	RESULT init_encode(const enum AVCodecID encodeid, AVFrame* frame);
	//
	RESULT start_video_encode(const AVFrame* frame);

	const AVFormatContext* get_avfctx() { return avfctx; }
private:

	AutoAVFormatContextPtr avfctx;
	AutoAVCodecContextPtr decode_ctx[2];
	AutoAVCodecContextPtr encode_ctx[2];
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

	//��������avfctx����Ƶ����Ƶ�����±�
	int AVStreamIndex[6];

	//��Ƶ��������Լ���Ƶ�� FrameQueue[AVMediaType]
	std::queue<AutoAVFramePtr> FrameQueue[6];
	std::mutex FrameQueue_mtx[6];
};