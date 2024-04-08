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
	LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context"; };
	~LearnVideo() {};
	RESULT open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	RESULT close();

	//初始化所有参数
	RESULT init();
	//初始化音视频解码器
	RESULT init_decode();
	//初始化音频转化器
	RESULT init_swr(const AVChannelLayout* out_ch_layout, const enum AVSampleFormat out_sample_fmt, const int out_sample_rate);
	//帧格式转化
	RESULT init_sws(const AVFrame* avf, const AVPixelFormat dstFormat, const int dstW = 0, const int dstH = 0);

	//开始转化图像帧
	RESULT start_sws(const AVFrame* avf);
	//开始音视频解码
	RESULT start_video_decode();
	//音视频编码
	RESULT init_encode(const enum AVCodecID encodeid, AVFrame* frame);

private:

	AutoAVFormatContextPtr avfctx;
	AutoAVCodecContextPtr decode_ctx[2];
	AutoAVCodecContextPtr encode_ctx[2];
	AutoSwrContextPtr swr_ctx;
	AutoSwsContextPtr sws_ctx;

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

	//用于存储avfctx的音频与视频流索引
	unsigned char AVStreamIndex[6];

	/*
	* 音频缓存队列以及音频锁 FrameQueue[AVMediaType] 使用并发队列()
	* 使用裸指针主要是因为容器try_pop是进行内存拷贝，不走构造函数
	* 接受指针时请使用智能指针 AutoAVFramePtr
	*/
	Concurrency::concurrent_queue<AVFrame*> FrameQueue[6];
};