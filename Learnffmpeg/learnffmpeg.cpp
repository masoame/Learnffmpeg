#include"learnffmpeg.h"

bool LearnVideo::open(const char* url, const AVInputFormat* fmt, AVDictionary** options)
{
	if (avformat_open_input(&avfctx, url, fmt, options)) return false;
	avformat_find_stream_info(avfctx, nullptr);
	av_dump_format(avfctx, 0, url, false);

	AVStreamIndex[AVMEDIA_TYPE_VIDEO] = av_find_best_stream(avfctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	AVStreamIndex[AVMEDIA_TYPE_AUDIO] = av_find_best_stream(avfctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);

	return true;
}

bool LearnVideo::close()
{
	decode_video_ctx = nullptr;
	encode_video_ctx = nullptr;

	return false;
}
bool LearnVideo::init()
{
	return init_decode();
}
//初始化解码器
bool LearnVideo::init_decode()
{
	decode_audio_ctx = avcodec_alloc_context3(nullptr);
	decode_video_ctx = avcodec_alloc_context3(nullptr);

	if (!decode_video_ctx || !decode_audio_ctx) return false;

	if(AVStreamIndex[AVMEDIA_TYPE_VIDEO]!= AVERROR_STREAM_NOT_FOUND)
	{
		if (avcodec_parameters_to_context(decode_video_ctx, avfctx->streams[AVStreamIndex[AVMEDIA_TYPE_VIDEO]]->codecpar) < 0)return false;
		decode_video = avcodec_find_decoder(decode_video_ctx->codec_id);
		if (avcodec_open2(decode_video_ctx, decode_video, NULL))return false;
	}

	if (AVStreamIndex[AVMEDIA_TYPE_AUDIO] != AVERROR_STREAM_NOT_FOUND)
	{
		if (avcodec_parameters_to_context(decode_audio_ctx, avfctx->streams[AVStreamIndex[AVMEDIA_TYPE_AUDIO]]->codecpar) < 0)return false;
		decode_audio = avcodec_find_decoder(decode_audio_ctx->codec_id);
		if (avcodec_open2(decode_audio_ctx, decode_audio, NULL))return false;
	}
	return true;
}

bool LearnVideo::init_swr(const AVChannelLayout* out_ch_layout,const enum AVSampleFormat out_sample_fmt,const int out_sample_rate)
{
	swr_ctx = swr_alloc();
	swr_alloc_set_opts2(&swr_ctx, out_ch_layout, out_sample_fmt, out_sample_rate, decode_audio->ch_layouts, *decode_audio->sample_fmts, *decode_audio->supported_samplerates, 0, nullptr);
	swr_init(swr_ctx);
	return swr_ctx;
}

bool LearnVideo::init_sws(const AVFrame* avf, const AVPixelFormat dstFormat,const int dstW,const int dstH)
{
	if (dstW == 0 || dstH == 0)
		sws_ctx = sws_getContext(avf->width, avf->height, (AVPixelFormat)avf->format, avf->width, avf->height, dstFormat, SWS_FAST_BILINEAR, nullptr, nullptr, 0);
	else 
		sws_ctx = sws_getContext(avf->width, avf->height, (AVPixelFormat)avf->format, dstW, dstH, dstFormat, SWS_FAST_BILINEAR, nullptr, nullptr, 0);
	return sws_ctx;
}

bool LearnVideo::start_video_decode(const std::function<bool(AVFrame*)>& video_action, const std::function<bool(AVFrame*)>& audio_action)
{
	int err;
	AutoAVPacketPtr avp = av_packet_alloc();
	AutoAVFramePtr avf = av_frame_alloc();

	const std::function<bool(AVFrame*)>* frame_action;
	AVCodecContext* decode_ctx = nullptr;

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
					AVERROR(ENOMEM);
					err = avcodec_receive_frame(decode_ctx, avf);
					if (err == 0) { if (*frame_action != nullptr) (*frame_action)(avf); }
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


bool LearnVideo::init_encode(const enum AVCodecID encodeid,AVFrame* frame)
{

	return true;
}

bool LearnVideo::start_video_encode(const AVFrame* frame)
{
	AVPacket* pkt_out = av_packet_alloc();
	//往编码器发送 AVFrame，然后不断读取 AVPacket
	int ret = avcodec_send_frame(encode_video_ctx, frame);
	if (ret < 0) {
		printf("avcodec_send_frame fail %d \n", ret);
		return ret;
	}
	while(true) 
	{
		ret = avcodec_receive_packet(encode_video_ctx, pkt_out);
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

void LearnVideo::start_sws(const AVFrame* src_avf, Channel_Planes* dst_ch)
{
	sws_scale(sws_ctx, src_avf->data, src_avf->linesize, 0, src_avf->height, dst_ch->data(), dst_ch->linesize);
}
