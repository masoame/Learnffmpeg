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

	//错误枚举
	enum RESULT
	{
		SUCCESS, UNKONW_ERROR, ARGS_ERROR,
		ALLOC_ERROR, OPEN_ERROR, INIT_ERROR,
		UNNEED_SWR
	};

	//sample_bit_size[AVSampleFormat(音频采样格式)] == 采样点的大小
	static inline const unsigned char sample_bit_size[13]{ 1,2,4,4,8,1,2,4,4,8,8,8,-1 };
	//planner转化为对应的packed
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

	//构造函数
	explicit LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context"; };
	~LearnVideo() {};

	//打开流
	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);

	//初始化您编解码器
	RESULT init_decode();

	//音频重采样(planner到packed格式转化)
	RESULT init_swr();
	RESULT sample_planner_to_packed(AVFrame* frame, uint8_t** data, int* linesize);

	//帧格式转化位RGB packed格式
	RESULT init_sws(const AVPixelFormat dstFormat, const int dstW = 0, const int dstH = 0);
	RESULT yuv_to_rgb_packed(uint8_t** data, int* linesize);

	//开始音视频解码
	RESULT start_decode_thread() noexcept;

	//插入队列
	void insert_queue(AVMediaType index, AutoAVFramePtr&& avf) noexcept;
	//从队列中取出并刷新工作指针的指向
	bool flush_frame(AVMediaType index) noexcept;

private:

	/*
	*  各种上下文指针
	*
	*	格式
	*	音频重采样
	*	格式化帧
	*	解码
	*	编码
	*
	*/

	AutoAVFormatContextPtr avfctx;
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	// AVStreamIndex[流的索引] == 流类型
	AVMediaType AVStreamIndex[6]{ AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN,AVMEDIA_TYPE_UNKNOWN };

	//并发队列
	using framedata_type = std::pair<AutoAVFramePtr, std::unique_ptr<char[]>>;
	using auto_framedata_type = std::pair<AutoAVFramePtr, char*>;

	Circular_Queue<framedata_type, 4> FrameQueue[6];
public:

	AutoAVCodecContextPtr decode_ctx[2];
	AutoAVCodecContextPtr encode_ctx[2];

	auto_framedata_type avframe_work[6];
	//insert_callback[AVMediaType(帧格式)] == 处理函数指针
	insert_callback_type insert_callback[6]{ 0 };
};