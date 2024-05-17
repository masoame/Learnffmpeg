#include "LearnSDL.h"

SDL_AudioFormat const LearnSDL::map_audio_formot[13]{
AUDIO_U8,
AUDIO_S16SYS,
AUDIO_S32SYS,
AUDIO_F32SYS,
-1,
AUDIO_U8,
AUDIO_S16SYS,
AUDIO_S32SYS,
AUDIO_F32SYS,
-1,
-1,
-1,
-1
};

const std::map<AVPixelFormat, SDL_PixelFormatEnum> LearnSDL::map_video_format
{
	{AV_PIX_FMT_YUV420P,SDL_PIXELFORMAT_IYUV},
	{AV_PIX_FMT_NV12,SDL_PIXELFORMAT_NV12},
	{AV_PIX_FMT_NV21,SDL_PIXELFORMAT_NV21}
};

SDL_Window* LearnSDL::sdl_win = nullptr;
SDL_Rect LearnSDL::rect;
SDL_Renderer* LearnSDL::sdl_renderer = nullptr;
SDL_Texture* LearnSDL::sdl_texture = nullptr;

Uint8* LearnSDL::audio_buf = new Uint8[sample_buf_size];
LearnVideo* LearnSDL::target = nullptr;
Uint8* LearnSDL::audio_pos = nullptr;
int LearnSDL::audio_buflen = 0;
bool LearnSDL::is_planner = false;

void LearnSDL::InitPlayer(LearnVideo& rely, const char* WindowName, SDL_AudioCallback callback)
{
	target = &rely;
	target->init_decode();
	target->insert_callback[AVMEDIA_TYPE_VIDEO] = convert_video_frame;
	target->insert_callback[AVMEDIA_TYPE_AUDIO] = convert_audio_frame;
	InitAudio(callback);

	target->start_decode_thread();


	InitVideo(WindowName);
}

void LearnSDL::StartPlayer()
{

	std::thread([&]
		{
			auto& video_ptr = target->avframe_work[AVMEDIA_TYPE_VIDEO];
			auto& audio_ptr = target->avframe_work[AVMEDIA_TYPE_VIDEO];
			AVRational& framerate = target->decode_ctx[AVMEDIA_TYPE_VIDEO]->framerate;
			float fps = framerate.num / static_cast<float>(framerate.den);
			DWORD delay = static_cast<DWORD>(1000 / fps - 1);

			SDL_PauseAudio(0);
			while (video_ptr.first != nullptr)
			{
				if (SDL_UpdateTexture(LearnSDL::sdl_texture, NULL, video_ptr.second, video_ptr.first->width))
				{
					std::cout << SDL_GetError() << std::endl;
					return -1;
				}
				if (SDL_RenderClear(LearnSDL::sdl_renderer))
				{
					std::cout << SDL_GetError() << std::endl;
					return -1;
				}
				
				if (SDL_RenderCopy(LearnSDL::sdl_renderer, LearnSDL::sdl_texture, NULL, &rect))
				{
					std::cout << SDL_GetError() << std::endl;
					return -1;
				}

				//渲染顯示操作
				SDL_RenderPresent(LearnSDL::sdl_renderer);
				target->flush_frame(AVMEDIA_TYPE_VIDEO);
				Sleep(delay);
				while (video_ptr.first->pts > audio_ptr.first->pts) {};
			}
		}).detach();
	
}

void LearnSDL::convert_video_frame(AVFrame* work, char*& buf) noexcept
{
	if (work == nullptr)return;
	if (buf == nullptr)buf = new char[work->width * work->height * 1.5];

	char* temp = buf;
	memcpy(temp, work->data[0], work->linesize[0] * work->height);
	temp += work->linesize[0] * work->height;
	memcpy(temp, work->data[1], work->linesize[1] * work->height / 2);
	temp += work->linesize[1] * work->height / 2;
	memcpy(temp, work->data[2], work->linesize[2] * work->height / 2);
}

void LearnSDL::convert_audio_frame(AVFrame* work, char*& buf) noexcept
{
	if (work == nullptr)return;
	if (buf == nullptr)buf = new char[work->linesize[0]];
	if (is_planner)
		target->sample_planner_to_packed(work, reinterpret_cast<uint8_t**>(&buf), &work->linesize[0]);
}

void SDLCALL LearnSDL::default_callback(void* userdata, Uint8* stream, int len)noexcept
{
	auto& audio_frame = target->avframe_work[AVMEDIA_TYPE_AUDIO];
	//清空流
	SDL_memset(stream, 0, len);
	if (audio_buflen == 0)
	{
		if (target->flush_frame(AVMEDIA_TYPE_AUDIO)) 
		{
			if (is_planner)audio_buf = reinterpret_cast<uint8_t*>(audio_frame.second);
			else audio_buf = audio_frame.first->data[0];

			audio_pos = audio_buf;
			audio_buflen = audio_frame.first->linesize[0];
		}
		else { SDL_CloseAudio(); return; }
	}

	len = audio_buflen > len ? len : audio_buflen;
	memcpy(stream, audio_pos, len);
	audio_pos += len;
	audio_buflen -= len;

	return;
}


void LearnSDL::InitAudio(SDL_AudioCallback callback)
{
	auto& audio_ctx = target->decode_ctx[AVMEDIA_TYPE_AUDIO];
	AVSampleFormat format = *audio_ctx->codec->sample_fmts;

	if (SDL_Init(SDL_INIT_AUDIO))throw "SDL_init error";
	
	if (av_sample_fmt_is_planar(format))
	{
		if (target->init_swr() != LearnVideo::SUCCESS) throw "init_swr() failed";
		is_planner = true;
	}

	if (map_audio_formot[format] == -1) throw "audio format is not suport!!!\n";

	SDL_AudioSpec sdl_audio{ 0 };
	sdl_audio.format = map_audio_formot[format];
	sdl_audio.channels = audio_ctx->ch_layout.nb_channels;
	sdl_audio.samples = audio_ctx->frame_size / audio_ctx->ch_layout.nb_channels;
	sdl_audio.silence = 0;
	sdl_audio.freq = audio_ctx->sample_rate;
	sdl_audio.callback = callback;

	if (SDL_OpenAudio(&sdl_audio, nullptr)) throw "SDL_OpenAudio failed!!!\n";
}

void LearnSDL::InitVideo(const char* title)
{
	LearnVideo::AutoAVFramePtr& video_frame = target->avframe_work[AVMEDIA_TYPE_VIDEO].first;

	if (SDL_Init(SDL_INIT_VIDEO)) throw "SDL_init error";
	if (!target->flush_frame(AVMEDIA_TYPE_VIDEO))throw "get_frame error";

	sdl_win = SDL_CreateWindow(title, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, video_frame->width, video_frame->height, SDL_WINDOW_FULLSCREEN);
	if (sdl_win == nullptr) throw "windows create error";

	sdl_renderer = SDL_CreateRenderer(sdl_win, -1, 0);
	if (sdl_renderer == nullptr)throw "Renderer create failed";

	const auto format = map_video_format.find(static_cast<AVPixelFormat>(video_frame->format));
	sdl_texture = SDL_CreateTexture(sdl_renderer, format->second, SDL_TEXTUREACCESS_STREAMING, video_frame->width, video_frame->height);
	if (sdl_texture == nullptr) throw "texture create failed";

	rect.x = 0;
	rect.y = 0;
	rect.w = video_frame->width;
	rect.h = video_frame->height;
}

