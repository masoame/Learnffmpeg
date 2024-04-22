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
		ALLOC_ERROR, OPEN_ERROR, INIT_ERROR,
		UNNEED_SWR
	};

	explicit LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context"; };
	~LearnVideo() {};

	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);

	RESULT init_decode();

	//音频重采样(planner到packed格式转化)
	RESULT init_swr(const AVFrame* avf);
	RESULT sample_planner_to_packed(const AVFrame* avf, uint8_t** data, int* linesize);

	//帧格式转化
	RESULT init_sws(const AVFrame* avf, const AVPixelFormat dstFormat, const int dstW = 0, const int dstH = 0);
	//开始转化图像帧
	RESULT start_sws(const AVFrame* avf);
	//开始音视频解码
	RESULT start_decode_thread() noexcept;
	//音视频编码
	RESULT init_encode(const enum AVCodecID encodeid, AVFrame* frame);

private:
	static AVSampleFormat map_palnner_to_packad[13];
	/*
	* 基础的解码编码需要的指针
	*/
	AutoAVFormatContextPtr avfctx;
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

	/*
	* 用于存储avfctx的音频与视频流索引
	*/

	AVMediaType AVStreamIndex[6]{ AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN ,AVMEDIA_TYPE_UNKNOWN,AVMEDIA_TYPE_UNKNOWN };

	/*
	* 音频缓存队列以及音频锁 FrameQueue[AVMediaType] 使用并发队列()
	* 使用裸指针主要是因为容器try_pop是进行内存拷贝，不走构造函数
	* 接受指针时请使用智能指针 AutoAVFramePtr
	*/

	/*
	* 线程id存储
	*/
	DWORD decode_thread_id = 0;

public:
	AutoAVCodecContextPtr decode_ctx[2];
	AutoAVCodecContextPtr encode_ctx[2];
	std::atomic_ushort QueueSize[6];
	Concurrency::concurrent_queue<AVFrame*> FrameQueue[6];
};