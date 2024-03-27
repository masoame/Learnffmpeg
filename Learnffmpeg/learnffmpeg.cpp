#include"learnffmpeg.h"

bool LearnVideo::open(const char* url, const AVInputFormat* fmt, AVDictionary** options)
{
	if (avformat_open_input(&avfctx, url, fmt, options)) return false;
	avformat_find_stream_info(avfctx, nullptr);
	av_dump_format(avfctx, 0, url, false);
	return true;
}

bool LearnVideo::close()
{
	decode_video_ctx = nullptr;
	encode_ctx = nullptr;

	return false;
}
//初始化解码器
bool LearnVideo::init_video_decode()
{
	//为画面和音频解码器分配空间
	decode_video_ctx = avcodec_alloc_context3(nullptr);
	decode_audio_ctx = avcodec_alloc_context3(nullptr);

	if (!decode_video_ctx || !decode_audio_ctx) return false;
	//遍历所有流找视频流
	int num;
	for (num = 0; num != avfctx->nb_streams; num++)
	{
		if (avfctx->streams[num]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
		{
			if (avcodec_parameters_to_context(decode_video_ctx, avfctx->streams[num]->codecpar) < 0)return false;
			decode_video = avcodec_find_decoder(decode_video_ctx->codec_id);
			if (avcodec_open2(decode_video_ctx, decode_video, NULL))return false;
		}
		else if (avfctx->streams[num]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			if (avcodec_parameters_to_context(decode_audio_ctx, avfctx->streams[num]->codecpar) < 0)return false;
			decode_audio = avcodec_find_decoder(decode_audio_ctx->codec_id);
			if (avcodec_open2(decode_audio_ctx, decode_audio, NULL))return false;
		}
	}



	return true;
}

bool LearnVideo::start_video_decode(const std::function<bool(AVFrame*)>& video_action, const std::function<bool(AVFrame*)>& audio_action)
{
	int err;
	AutoAVPacketPtr avp = av_packet_alloc();
	AutoAVFramePtr avf = av_frame_alloc();

	const std::function<bool(AVFrame*)>* frame_action;
	AVCodecContext* decode_ctx=nullptr;

	while (true)
	{
		err = av_read_frame(avfctx, avp);
		if (avp->stream_index == AVMEDIA_TYPE_VIDEO || avp->stream_index == AVMEDIA_TYPE_AUDIO)
		{
			if (avp->stream_index == AVMEDIA_TYPE_VIDEO) { decode_ctx = decode_video_ctx; frame_action = &video_action; }
			else { decode_ctx = decode_audio_ctx; frame_action = &audio_action;}

			if (err == AVERROR_EOF)
			{
				avcodec_send_packet(decode_ctx, avp);
				av_packet_unref(avp);
				while (true)
				{
					err = avcodec_receive_frame(decode_ctx, avf);
					if (err == 0) { if (*frame_action != nullptr) (*frame_action)(avf); }
					else if (err == AVERROR_EOF) { goto DecodeEND; }
					else return false;
				}
			}
			else if (err == 0)
			{
				while ((err = avcodec_send_packet(decode_ctx, avp)) == AVERROR(EAGAIN)) { Sleep(10); }
				av_packet_unref(avp);
				while (true)
				{
					err = avcodec_receive_frame(decode_ctx, avf);
					if (err == 0) { if (frame_action != nullptr) (*frame_action)(avf); }
					else if (err == AVERROR(EAGAIN)) break;
					else if (err == AVERROR_EOF) { goto DecodeEND; }
					else return false;
				}
			}
		}
		else { av_packet_unref(avp); }
	}
	DecodeEND:
	return true;
}


bool LearnVideo::init_video_encode(const enum AVCodecID encodeid,AVFrame* frame)
{
	encodec = avcodec_find_encoder(encodeid);
	encode_ctx = avcodec_alloc_context3(encodec);
	encode_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	encode_ctx->bit_rate = 400000;
	encode_ctx->framerate = decode_video_ctx->framerate;
	encode_ctx->gop_size = 10;
	encode_ctx->max_b_frames = 5;
	encode_ctx->profile = FF_PROFILE_H264_MAIN;

	//编码器的时间基要取 AVFrame 的时间基，因为 AVFrame 是输入。
	encode_ctx->time_base = avfctx->streams[0]->time_base;
	encode_ctx->width = avfctx->streams[0]->codecpar->width;
	encode_ctx->height = avfctx->streams[0]->codecpar->height;
	encode_ctx->sample_aspect_ratio = frame->sample_aspect_ratio;
	encode_ctx->pix_fmt = (AVPixelFormat)frame->format;
	encode_ctx->color_range = frame->color_range;
	encode_ctx->color_primaries = frame->color_primaries;
	encode_ctx->color_trc = frame->color_trc;
	encode_ctx->colorspace = frame->colorspace;
	encode_ctx->chroma_sample_location = frame->chroma_location;

	encode_ctx->field_order = AV_FIELD_PROGRESSIVE;
	int err;
	if ((err = avcodec_open2(encode_ctx, encodec, NULL)) < 0) {
		std::cout << "open codec faile " << err << std::endl;
		return false;
	}
	return true;
}

bool LearnVideo::start_video_encode(const AVFrame* frame)
{
	AVPacket* pkt_out = av_packet_alloc();
	//往编码器发送 AVFrame，然后不断读取 AVPacket
	int ret = avcodec_send_frame(encode_ctx, frame);
	if (ret < 0) {
		printf("avcodec_send_frame fail %d \n", ret);
		return ret;
	}
	while(true) 
	{
		ret = avcodec_receive_packet(encode_ctx, pkt_out);
		if (ret == AVERROR(EAGAIN)) {
			break;
		}
		//前面没有往 编码器发 NULL,所以正常情况 ret 不会小于 0
		if (ret < 0) {
			printf("avcodec_receive_packet fail %d \n", ret);
			return ret;
		}
		//编码出 AVPacket ，打印一些信息。
		printf("pkt_out size : %d \n", pkt_out->size);

		av_packet_unref(pkt_out);
	}

	return false;
}

std::unique_ptr<unsigned char[]> LearnVideo::YUV2RGB(const AVFrame* avf,const char* filepath,AVPixelFormat dstFormat)
{
	const int RBG_size = avf->width * avf->height * 3;
	std::unique_ptr<unsigned char[]> buf{new unsigned char[RBG_size] {0}};

	//创建转换上下文
	SwsContext* swsc = sws_getContext(avf->width, avf->height, (AVPixelFormat)avf->format, avf->width, avf->height, dstFormat, SWS_FAST_BILINEAR, nullptr, nullptr, 0);
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	//设置RGB通道
	data[0] = buf.get();
	int lines[AV_NUM_DATA_POINTERS] = { 0 };
	//设置通道大小
	lines[0] = avf->width * 3;
	//开始转换
	sws_scale(swsc, avf->data, avf->linesize, 0, avf->height, data, lines);

	sws_freeContext(swsc);
	return buf;
}
