#include"learnffmpeg.h"

bool LearnVideo::open(const char* url, const AVInputFormat* fmt, AVDictionary** options)
{
	if (avformat_open_input(&avfctx, url, fmt, options)) return false;
	avformat_find_stream_info(avfctx, nullptr);
	//第二三参数直接输出在打印第一行，不影响AVFormatContext结构体信息
	av_dump_format(avfctx, 0, url, false);

	//std::cout << avfctx->duration/(AV_TIME_BASE/1000)/60000.0f << std::endl;
	return true;
}

bool LearnVideo::close()
{
	//释放编解码器上下文
	avcodec_free_context(&this->decode_ctx);
	avcodec_free_context(&this->encode_ctx);
	return false;
}
//初始化解码器
bool LearnVideo::init_video_decode(const AVCodec* codec)
{
	//为解码器分配空间
	decode_ctx = avcodec_alloc_context3(codec);
	if (!decode_ctx) return false;
	//遍历所有流找视频流
	int num;
	for (num = 0; num != avfctx->nb_streams; num++)
		if (avfctx->streams[num]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) break;
	//有遍历完未有视频流
	if (num == avfctx->nb_streams) return false;
	//设置解码器信息
	if (avcodec_parameters_to_context(decode_ctx, avfctx->streams[num]->codecpar) < 0)return false;
	//寻找视频对应的编码
	decodec = avcodec_find_decoder(decode_ctx->codec_id);
	//设置视频编码
	if(avcodec_open2(decode_ctx, decodec, NULL))return false;

	return true;
}

bool LearnVideo::start_video_decode(const std::function<bool(AVFrame*)>& frame_action)
{
	int err;
	AVPacket* avp = av_packet_alloc();
	AVFrame* avf = av_frame_alloc();
	for (bool read_end = true; read_end;)
	{
		//读取包
		err = av_read_frame(avfctx, avp);
		//读取到包为音频包
		if (avp->stream_index == AVMEDIA_TYPE_AUDIO)
		{
			av_packet_unref(avp);
			continue;
		}
		//读取到文件尾部 (avp的data和size为null)
		if (err == AVERROR_EOF)
		{
			//冲刷解码器
			err = avcodec_send_packet(decode_ctx, avp);
			av_packet_unref(avp);

			while (true)
			{
				err = avcodec_receive_frame(decode_ctx, avf);

				if (err == AVERROR_EOF) { read_end = false; break; }
				else if (err == 0)
				{
					if (frame_action != nullptr) frame_action(avf);
				}
			}
		}
		else if (err == 0)
		{
			while ((err = avcodec_send_packet(decode_ctx, avp)) == AVERROR(EAGAIN))
			{
				std::cout << "AVERROR(EAGAIN)" << std::endl;
				Sleep(10);
			}
			av_packet_unref(avp);

			while (true)
			{
				err = avcodec_receive_frame(decode_ctx, avf);
				if (err == AVERROR(EAGAIN)) break;
				else if (err == AVERROR_EOF) { read_end = false; break; }
				else if (err == 0)
				{
					//got a frame after
					if (frame_action != nullptr) frame_action(avf);
					av_frame_unref(avf);
				}
				else throw "unkonw error!!!!";
			}

		}
	}
	av_frame_free(&avf);
	av_packet_free(&avp);
	return true;
}

bool LearnVideo::init_video_encode(const enum AVCodecID encodeid,AVFrame* frame)
{
	encodec = avcodec_find_encoder(encodeid);
	encode_ctx = avcodec_alloc_context3(encodec);
	encode_ctx->codec_type = AVMEDIA_TYPE_VIDEO;
	encode_ctx->bit_rate = 400000;
	encode_ctx->framerate = decode_ctx->framerate;
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
