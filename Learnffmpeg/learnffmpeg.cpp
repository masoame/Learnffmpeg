#include"learnffmpeg.h"

bool LearnVideo::open(const char* url, const AVInputFormat* fmt, AVDictionary** options)
{
	if (avformat_open_input(&avfctx, url, fmt, options)) return false;
	avformat_find_stream_info(avfctx, nullptr);
	//�ڶ�������ֱ������ڴ�ӡ��һ�У���Ӱ��AVFormatContext�ṹ����Ϣ
	av_dump_format(avfctx, 0, url, false);

	//std::cout << avfctx->duration/(AV_TIME_BASE/1000)/60000.0f << std::endl;
	return true;
}

bool LearnVideo::close()
{
	//�ͷű������������
	avcodec_free_context(&this->decode_ctx);
	avcodec_free_context(&this->encode_ctx);
	return false;
}
//��ʼ��������
bool LearnVideo::init_video_decode(const AVCodec* codec)
{
	//Ϊ����������ռ�
	decode_ctx = avcodec_alloc_context3(codec);
	if (!decode_ctx) return false;
	//��������������Ƶ��
	int num;
	for (num = 0; num != avfctx->nb_streams; num++)
		if (avfctx->streams[num]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) break;
	//�б�����δ����Ƶ��
	if (num == avfctx->nb_streams) return false;
	//���ý�������Ϣ
	if (avcodec_parameters_to_context(decode_ctx, avfctx->streams[num]->codecpar) < 0)return false;
	//Ѱ����Ƶ��Ӧ�ı���
	decodec = avcodec_find_decoder(decode_ctx->codec_id);
	//������Ƶ����
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
		//��ȡ��
		err = av_read_frame(avfctx, avp);
		//��ȡ����Ϊ��Ƶ��
		if (avp->stream_index == AVMEDIA_TYPE_AUDIO)
		{
			av_packet_unref(avp);
			continue;
		}
		//��ȡ���ļ�β�� (avp��data��sizeΪnull)
		if (err == AVERROR_EOF)
		{
			//��ˢ������
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

	//��������ʱ���Ҫȡ AVFrame ��ʱ�������Ϊ AVFrame �����롣
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
	//������������ AVFrame��Ȼ�󲻶϶�ȡ AVPacket
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
		//ǰ��û���� �������� NULL,����������� ret ����С�� 0
		if (ret < 0) {
			printf("avcodec_receive_packet fail %d \n", ret);
			return ret;
		}
		//����� AVPacket ����ӡһЩ��Ϣ��
		printf("pkt_out size : %d \n", pkt_out->size);

		av_packet_unref(pkt_out);
	}

	return false;
}

std::unique_ptr<unsigned char[]> LearnVideo::YUV2RGB(const AVFrame* avf,const char* filepath,AVPixelFormat dstFormat)
{
	const int RBG_size = avf->width * avf->height * 3;
	std::unique_ptr<unsigned char[]> buf{new unsigned char[RBG_size] {0}};

	//����ת��������
	SwsContext* swsc = sws_getContext(avf->width, avf->height, (AVPixelFormat)avf->format, avf->width, avf->height, dstFormat, SWS_FAST_BILINEAR, nullptr, nullptr, 0);
	uint8_t* data[AV_NUM_DATA_POINTERS] = { 0 };
	//����RGBͨ��
	data[0] = buf.get();
	int lines[AV_NUM_DATA_POINTERS] = { 0 };
	//����ͨ����С
	lines[0] = avf->width * 3;
	//��ʼת��
	sws_scale(swsc, avf->data, avf->linesize, 0, avf->height, data, lines);

	sws_freeContext(swsc);
	return buf;
}
