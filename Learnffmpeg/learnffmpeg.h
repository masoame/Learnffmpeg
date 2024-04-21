#pragma once
#include"common.hpp"
#include <concurrent_queue.h>

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
	/*
	*
	*/
	explicit LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context"; };
	~LearnVideo() {};
	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	RESULT close();

	//��ʼ�����в���
	RESULT init();
	//��ʼ������Ƶ������
	RESULT init_decode();

	void sample_planner_to_packed(const AVFrame* avf, uint8_t** data, int* linesize);

	//��ʼ����Ƶת����
	RESULT init_swr(const AVChannelLayout* out_ch_layout, const enum AVSampleFormat out_sample_fmt, const int out_sample_rate);
	//֡��ʽת��
	RESULT init_sws(const AVFrame* avf, const AVPixelFormat dstFormat, const int dstW = 0, const int dstH = 0);

	//��ʼת��ͼ��֡
	RESULT start_sws(const AVFrame* avf);
	//��ʼ����Ƶ����
	RESULT start_decode_thread() noexcept;
	//����Ƶ����
	RESULT init_encode(const enum AVCodecID encodeid, AVFrame* frame);

private:

	/*
	* �����Ľ��������Ҫ��ָ��
	*/
	AutoAVFormatContextPtr avfctx;
	AutoAVCodecContextPtr decode_ctx[2];
	AutoAVCodecContextPtr encode_ctx[2];
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

	/*
	* ���ڴ洢avfctx����Ƶ����Ƶ������
	*/

	AVMediaType AVStreamIndex[6]{ AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN,AVMEDIA_TYPE_UNKNOWN };

	/*
	* ��Ƶ��������Լ���Ƶ�� FrameQueue[AVMediaType] ʹ�ò�������()
	* ʹ����ָ����Ҫ����Ϊ����try_pop�ǽ����ڴ濽�������߹��캯��
	* ����ָ��ʱ��ʹ������ָ�� AutoAVFramePtr
	*/

	/*
	* �߳�id�洢
	*/
	DWORD decode_thread_id = 0;

public:
	std::atomic_ushort QueueSize[6];
	Concurrency::concurrent_queue<AVFrame*> FrameQueue[6];
};