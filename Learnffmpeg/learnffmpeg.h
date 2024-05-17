#pragma once
#include"common.hpp"

class LearnVideo
{
public:

	using AutoAVPacketPtr = AutoPtr<AVPacket, Functor<av_packet_free>, true>;
	using AutoAVCodecContextPtr = AutoPtr<AVCodecContext, Functor<avcodec_free_context>, true>;
	using AutoAVFormatContextPtr = AutoPtr<AVFormatContext, Functor<avformat_free_context>, false>;
	using AutoSwsContextPtr = AutoPtr<SwsContext, Functor<sws_freeContext>, false>;
	using AutoSwrContextPtr = AutoPtr<SwrContext, Functor<swr_free>, true>;
	using AutoAVFramePtr = AutoPtr<AVFrame, Functor<av_frame_free>, true>;

	using insert_callback_type = void (*)(AVFrame*, char*& buf) noexcept;

	//����ö��
	enum RESULT
	{
		SUCCESS, UNKONW_ERROR, ARGS_ERROR,
		ALLOC_ERROR, OPEN_ERROR, INIT_ERROR,
		UNNEED_SWR
	};

	//sample_bit_size[AVSampleFormat(��Ƶ������ʽ)] == ������Ĵ�С
	static inline const unsigned char sample_bit_size[13]{ 1,2,4,4,8,1,2,4,4,8,8,8,-1 };
	//plannerת��Ϊ��Ӧ��packed
	static inline const AVSampleFormat map_palnner_to_packad[13]
	{
	AV_SAMPLE_FMT_NONE,
	AV_SAMPLE_FMT_NONE,
	AV_SAMPLE_FMT_NONE,
	AV_SAMPLE_FMT_NONE,
	AV_SAMPLE_FMT_NONE,
	AV_SAMPLE_FMT_U8,
	AV_SAMPLE_FMT_S16,
	AV_SAMPLE_FMT_S32,
	AV_SAMPLE_FMT_FLT,
	AV_SAMPLE_FMT_DBL,
	AV_SAMPLE_FMT_NONE,
	AV_SAMPLE_FMT_S64,
	AV_SAMPLE_FMT_NONE
	};

	//���캯��
	explicit LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context"; };
	~LearnVideo() {};

	//����
	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);

	//��ʼ�����������
	RESULT init_decode();

	//��Ƶ�ز���(planner��packed��ʽת��)
	RESULT init_swr();
	RESULT sample_planner_to_packed(AVFrame* frame, uint8_t** data, int* linesize);

	//֡��ʽת��λRGB packed��ʽ
	RESULT init_sws(const AVPixelFormat dstFormat, const int dstW = 0, const int dstH = 0);
	RESULT yuv_to_rgb_packed(uint8_t** data, int* linesize);

	//��ʼ����Ƶ����
	RESULT start_decode_thread() noexcept;

	//�������
	void insert_queue(AVMediaType index, AutoAVFramePtr&& avf) noexcept;
	//�Ӷ�����ȡ����ˢ�¹���ָ���ָ��
	bool flush_frame(AVMediaType index) noexcept;

private:

	/*
	*  ����������ָ��
	*
	*	��ʽ
	*	��Ƶ�ز���
	*	��ʽ��֡
	*	����
	*	����
	*
	*/

	AutoAVFormatContextPtr avfctx;
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	// AVStreamIndex[��������] == ������
	AVMediaType AVStreamIndex[6]{ AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN,AVMEDIA_TYPE_UNKNOWN };

	//��������
	using framedata_type = std::pair<AutoAVFramePtr, std::unique_ptr<char[]>>;
	using auto_framedata_type = std::pair<AutoAVFramePtr, char*>;

	Circular_Queue<framedata_type, 4> FrameQueue[6];
public:

	AutoAVCodecContextPtr decode_ctx[2];
	AutoAVCodecContextPtr encode_ctx[2];

	auto_framedata_type avframe_work[6];
	//insert_callback[AVMediaType(֡��ʽ)] == ������ָ��
	insert_callback_type insert_callback[6]{ 0 };
};