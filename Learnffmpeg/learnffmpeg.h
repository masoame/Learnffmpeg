#pragma once
#include"common.h"

class LearnVideo
{
public:
	LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context";  };
	~LearnVideo() {};
	bool open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	bool close();
	
	bool init_video_decode();
	bool start_video_decode(const std::function<bool(AVFrame*)>& video_action = nullptr, const std::function<bool(AVFrame*)>& audio_action = nullptr);


	bool init_video_encode(const enum AVCodecID encodeid, AVFrame* frame);
	bool start_video_encode(const AVFrame* frame);


	static std::unique_ptr<unsigned char[]> YUV2RGB(const AVFrame*, const char*, AVPixelFormat);

	const AVFormatContext *  get_avfctx() { return avfctx; }
private:
	AutoAVFormatContextPtr avfctx;
	AutoAVCodecContextPtr decode_video_ctx, decode_audio_ctx, encode_ctx;
	AutoSwrContextPtr swr_ctx;

	const AVCodec* decode_video = nullptr, * decode_audio = nullptr, * encodec = nullptr;

};