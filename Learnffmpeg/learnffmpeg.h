#pragma once
#include"common.hpp"
#include <concurrent_queue.h>

class LearnVideo
{
public:
	using AutoAVPacketPtr = AutoPtr<AVPacket, Functor<av_packet_free>, true>;
	using AutoAVCodecContextPtr = AutoPtr<AVCodecContext, Functor<avcodec_free_context>, true>;
	using AutoAVFormatContextPtr = AutoPtr<AVFormatContext, Functor<avformat_free_context>, false>;
	using AutoSwsContextPtr = AutoPtr<SwsContext, Functor<sws_freeContext>, false>;
	using AutoSwrContextPtr = AutoPtr<SwrContext, Functor<swr_free>, true>;
	using AutoAVFramePtr = AutoPtr<AVFrame, Functor<av_frame_free>, true>;

	enum RESULT
	{
		SUCCESS, UNKONW_ERROR, ARGS_ERROR,
		ALLOC_ERROR, OPEN_ERROR, INIT_ERROR,
		UNNEED_SWR
	};

	//构造函数
	explicit LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context"; };
	~LearnVideo() {};

	//打开流
	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);

	//初始化您编解码器
	RESULT init_decode();
	RESULT init_encode(const enum AVCodecID encodeid, AVFrame* frame);

	//音频重采样(planner到packed格式转化)
	RESULT init_swr(const AVFrame* avf);
	RESULT sample_planner_to_packed(const AVFrame* avf , uint8_t** data, int* linesize);

	//帧格式转化
	RESULT init_sws(const AVFrame* avf, const AVPixelFormat dstFormat, const int dstW = 0, const int dstH = 0);
	RESULT yuv_to_rgb(const AVFrame* avf,uint8_t** data, int* linesize);

	//开始音视频解码
	RESULT start_decode_thread() noexcept;



private:

	AutoAVFormatContextPtr avfctx;
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;
	AutoAVCodecContextPtr decode_ctx[2];
	AutoAVCodecContextPtr encode_ctx[2];

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

	/*
	* 用于存储avfctx的音频与视频流索引
	*/

	AVMediaType AVStreamIndex[6]{ AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN,AVMEDIA_TYPE_UNKNOWN };

	/*
	* 线程id存储
	*/
	DWORD decode_thread_id = 0;

public:

	//每个采样点的占字节数
	static unsigned char sample_bit_size[13];

	static AVSampleFormat map_palnner_to_packad[13];

	class 
	{
		std::atomic_ushort size[6];
		Concurrency::concurrent_queue<AVFrame*> FrameQueue[6];

	public:
		AutoAVFramePtr avframe_work[6];
		inline void insert_queue(AVMediaType index, AutoAVFramePtr && avf) noexcept
		{
			while (size[index] == 50) Sleep(1);
			FrameQueue[index].push(avf.release());
			avf = av_frame_alloc();
			size[index]++;
		}

		/*
		* if return flase status is eof 
		*/

		inline bool flush_frame(AVMediaType index) noexcept
		{
			avframe_work[index].reset();

			while (FrameQueue[index].empty()) Sleep(1);
			size[index]--;
			FrameQueue[index].try_pop(avframe_work[index]);

			if (avframe_work[index] == nullptr)return false;
			return true;
		}
	}QueueFrame;
	


};