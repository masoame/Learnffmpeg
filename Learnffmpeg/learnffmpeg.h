#pragma once
#include"common.h"

class LearnVideo
{
public:
	LearnVideo() :avfctx(avformat_alloc_context()) { if (!avfctx) throw "function error: avformat_alloc_context";  };
	~LearnVideo() { avformat_free_context(avfctx); avfctx = nullptr; };
	bool open(const char* url, const AVInputFormat* fmt = nullptr, AVDictionary** options = nullptr);
	bool close();
	bool init_video_decode(const AVCodec* codec = nullptr);
	bool start_video_decode(const std::function<bool(AVFrame*)>& frame_action = nullptr);
	bool init_video_encode(const enum AVCodecID encodeid, AVFrame* frame);
	bool start_video_encode(const AVFrame* frame);


	static std::unique_ptr<unsigned char[]> YUV2RGB(const AVFrame*, const char*, AVPixelFormat);

	const AVFormatContext *  get_avfctx() { return avfctx; }
private:
	AVFormatContext* avfctx = nullptr;
	AVCodecContext* decode_ctx = nullptr, * encode_ctx = nullptr;
	const AVCodec* decodec = nullptr, * encodec = nullptr;

};