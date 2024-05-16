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
	target->start_decode_thread();

	InitAudio(callback);
	InitVideo(WindowName);
}

void LearnSDL::StartPlayer()
{
	std::thread([&]
		{
			auto& work = target->avframe_work[AVMEDIA_TYPE_VIDEO];
			SDL_PauseAudio(0);
			while (work.first != nullptr)
			{
				if (SDL_UpdateTexture(LearnSDL::sdl_texture, NULL, work.second, work.first->width))
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
				Sleep(30);
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

	LearnVideo::AutoAVFramePtr& audio_frame = target->avframe_work[AVMEDIA_TYPE_AUDIO].first;

	if (is_planner)
	{
		audio_buflen = sample_buf_size;
		target->sample_planner_to_packed(&audio_buf, &audio_buflen);
		audio_pos = audio_buf;
	}
	else
	{
		audio_pos = audio_frame->data[0];
		audio_buflen = audio_frame->nb_samples;
	}
}

bool LearnSDL::format_frame() noexcept
{
	LearnVideo::AutoAVFramePtr& audio_frame = target->avframe_work[AVMEDIA_TYPE_AUDIO].first;

	if (audio_frame == nullptr) return false;
	if (is_planner)
	{
		audio_buflen = sample_buf_size;
		target->sample_planner_to_packed(&audio_buf, &audio_buflen);
		audio_pos = audio_buf;
	}
	else
	{
		audio_pos = audio_frame->data[0];
		audio_buflen = audio_frame->nb_samples;
	}
	return true;
}

void SDLCALL LearnSDL::default_callback(void* userdata, Uint8* stream, int len)noexcept
{
	//清空流
	SDL_memset(stream, 0, len);
	if (audio_buflen == 0)
	{
		if (target->flush_frame(AVMEDIA_TYPE_AUDIO) && format_frame()) {}
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
	LearnVideo::AutoAVFramePtr& audio_frame = target->avframe_work[AVMEDIA_TYPE_AUDIO].first;

	if (SDL_Init(SDL_INIT_AUDIO))throw "SDL_init error";

	if (!target->flush_frame(AVMEDIA_TYPE_AUDIO))throw "get_frame error";
	if (av_sample_fmt_is_planar((AVSampleFormat)audio_frame->format))
	{
		if (target->init_swr() != LearnVideo::SUCCESS) throw "init_swr() failed";
		is_planner = true;
	}
	if (!format_frame())throw "format_frame error";

	if (map_audio_formot[audio_frame->format] == -1) throw "audio format is not suport!!!\n";

	SDL_AudioSpec sdl_audio{ 0 };
	sdl_audio.format = map_audio_formot[audio_frame->format];
	sdl_audio.channels = audio_frame->ch_layout.nb_channels;
	sdl_audio.samples = audio_frame->linesize[0] / LearnVideo::sample_bit_size[audio_frame->format] / audio_frame->ch_layout.nb_channels;
	sdl_audio.silence = 0;
	sdl_audio.freq = audio_frame->sample_rate;
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

