#include"learnffmpeg.h"

//转化map
AVSampleFormat LearnVideo::map_palnner_to_packad[13]{
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


LearnVideo::RESULT LearnVideo::open(const char* url, const AVInputFormat* fmt, AVDictionary** options)
{
	if (avformat_open_input(&avfctx, url, fmt, options)) return OPEN_ERROR;
	avformat_find_stream_info(avfctx, nullptr);
	av_dump_format(avfctx, 0, url, false);

	int index = av_find_best_stream(avfctx, AVMEDIA_TYPE_VIDEO, -1, -1, NULL, 0);
	if (index != AVERROR_STREAM_NOT_FOUND) AVStreamIndex[index] = AVMEDIA_TYPE_VIDEO;
	index = av_find_best_stream(avfctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
	if (index != AVERROR_STREAM_NOT_FOUND) AVStreamIndex[index] = AVMEDIA_TYPE_AUDIO;

	return SUCCESS;
}


LearnVideo::RESULT LearnVideo::init_decode()
{
	decode_ctx[AVMEDIA_TYPE_VIDEO] = avcodec_alloc_context3(nullptr);
	decode_ctx[AVMEDIA_TYPE_AUDIO] = avcodec_alloc_context3(nullptr);

	if (!decode_ctx[AVMEDIA_TYPE_VIDEO] || !decode_ctx[AVMEDIA_TYPE_AUDIO]) return ALLOC_ERROR;


	for (int i = 0; i != 6; i++)
	{
		if (AVStreamIndex[i] == AVMEDIA_TYPE_UNKNOWN)continue;
		if (avcodec_parameters_to_context(decode_ctx[AVStreamIndex[i]], avfctx->streams[i]->codecpar) < 0)return ARGS_ERROR;
		decode_video = avcodec_find_decoder(decode_ctx[AVStreamIndex[i]]->codec_id);
		if (avcodec_open2(decode_ctx[AVStreamIndex[i]], decode_video, NULL))return OPEN_ERROR;
	}

	return SUCCESS;
}

LearnVideo::RESULT LearnVideo::init_swr(const AVFrame* avf)
{
	if (avf == nullptr)return ARGS_ERROR;

	if (avf->format == AV_SAMPLE_FMT_NONE || map_palnner_to_packad[avf->format] == AV_SAMPLE_FMT_NONE)return UNNEED_SWR;

	swr_ctx = swr_alloc();
	if (!swr_ctx)return ALLOC_ERROR;

	AVChannelLayout out_ch_layout;
	out_ch_layout.nb_channels = 1;
	out_ch_layout.order = AV_CHANNEL_ORDER_NATIVE;
	out_ch_layout.u.mask = 1;
	out_ch_layout.opaque = nullptr;
	
	if (swr_alloc_set_opts2(&swr_ctx, &out_ch_layout, map_palnner_to_packad[avf->format], avf->sample_rate, &avf->ch_layout, (AVSampleFormat)avf->format, avf->sample_rate, 0, nullptr))return UNKONW_ERROR;
	if (swr_init(swr_ctx))return INIT_ERROR;
	return SUCCESS;
}

LearnVideo::RESULT LearnVideo::sample_planner_to_packed(const AVFrame* avf, uint8_t** data, int* linesize)
{
	int temp = avf->ch_layout.nb_channels * avf->linesize[0];
	*linesize = swr_convert(swr_ctx, data, temp, avf->data, avf->linesize[0]);
	return SUCCESS;
}

LearnVideo::RESULT LearnVideo::init_sws(const AVFrame* avf, const AVPixelFormat dstFormat, const int dstW, const int dstH)
{
	if (dstW == 0 || dstH == 0)
		sws_ctx = sws_getContext(avf->width, avf->height, (AVPixelFormat)avf->format, avf->width, avf->height, dstFormat, SWS_FAST_BILINEAR, nullptr, nullptr, 0);
	else
		sws_ctx = sws_getContext(avf->width, avf->height, (AVPixelFormat)avf->format, dstW, dstH, dstFormat, SWS_FAST_BILINEAR, nullptr, nullptr, 0);
	return SUCCESS;
}

LearnVideo::RESULT LearnVideo::start_decode_thread() noexcept
{
	std::thread([&]()->void
	{
		decode_thread_id = GetCurrentThreadId();
		int err;
		AutoAVPacketPtr avp = av_packet_alloc();
		AutoAVFramePtr avf = av_frame_alloc();

		while (true)
		{
			err = av_read_frame(avfctx, avp);

			AVMediaType index = AVStreamIndex[avp->stream_index];
			if (index == AVMEDIA_TYPE_VIDEO || index == AVMEDIA_TYPE_AUDIO)
			{
				if (err == AVERROR_EOF)
				{
					avcodec_send_packet(decode_ctx[index], avp);
					av_packet_unref(avp);
					while (true)
					{
						err = avcodec_receive_frame(decode_ctx[index], avf);
						if (err == 0) {
							while (QueueSize[index] == 10)Sleep(5);

							FrameQueue[index].push(avf.release());
							avf = av_frame_alloc();
							QueueSize[index]++;
						}
						else if (err == AVERROR_EOF) return;
						//未知错误暂时不处理
						else return;
					}
				}
				else if (err == 0)
				{
					while ((err = avcodec_send_packet(decode_ctx[index], avp)) == AVERROR(EAGAIN)) { Sleep(1); }
					av_packet_unref(avp);
					while (true)
					{
						AVERROR(ENOMEM);
						err = avcodec_receive_frame(decode_ctx[index], avf);
						if (err == 0) {
							while (QueueSize[index] == 75)Sleep(10);

							FrameQueue[index].push(avf.release());
							avf = av_frame_alloc();

							QueueSize[index]++;
						}
						else if (err == AVERROR(EAGAIN)) break;
						else if (err == AVERROR_EOF) return;
						//未知错误暂时不处理
						else return;
					}
				}
			}
			else { av_packet_unref(avp); }
		}
	}).detach();
	return SUCCESS;
}

LearnVideo::RESULT LearnVideo::init_encode(const enum AVCodecID encodeid, AVFrame* frame)
{
	return SUCCESS;
}

