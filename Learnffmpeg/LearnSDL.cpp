#include "LearnSDL.h"

SDL_AudioFormat LearnSDL::map_audio_formot[13]{
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

Uint8* LearnSDL::audio_buf = new Uint8[sample_buf_size];
LearnVideo* LearnSDL::target = nullptr;
Uint8* LearnSDL::audio_pos = nullptr;
int LearnSDL::buflen = 0;
AutoAVFramePtr LearnSDL::avf = nullptr;
bool LearnSDL::is_planner = false;

bool LearnSDL::flush_buf() noexcept
{
	avf.reset();
	while (target->FrameQueue[AVMEDIA_TYPE_AUDIO].empty()) Sleep(1);
	target->FrameQueue[AVMEDIA_TYPE_AUDIO].try_pop(avf);
	target->QueueSize[AVMEDIA_TYPE_AUDIO]--;

	if (avf == nullptr)return false;
	return true;
}

bool LearnSDL::format_frame() noexcept
{
	if (avf == nullptr) return false;

	if (is_planner)
	{
		buflen = sample_buf_size;
		target->sample_planner_to_packed(avf, &audio_buf, &buflen);
		audio_pos = audio_buf;
	}
	else
	{
		audio_pos = avf->data[0];
		buflen = avf->nb_samples;
	}
	return true;
}

void SDLCALL LearnSDL::default_callback(void* userdata, Uint8* stream, int len)noexcept
{
	//Çå¿ÕÁ÷
	SDL_memset(stream, 0, len);
	if (buflen == 0)
	{
		if (flush_buf() && format_frame()) {}
		else { SDL_CloseAudio(); return; }
	}

	len = buflen > len ? len : buflen;
	memcpy(stream, audio_pos, len);
	audio_pos += len;
	buflen -= len;

	return;
}
void LearnSDL::InitAudio(SDL_AudioCallback callback)
{
	if (SDL_Init(SDL_INIT_AUDIO))throw "SDL_init error";

	if (!flush_buf())throw "get_frame error";
	if (av_sample_fmt_is_planar((AVSampleFormat)avf->format))
	{
		if (target->init_swr(avf) != LearnVideo::SUCCESS) throw "init_swr() failed";
		is_planner = true;
	}
	if (!format_frame())throw "format_frame error";

	if (map_audio_formot[avf->format] == -1) throw "audio format is not suport!!!\n";

	SDL_AudioSpec sdl_audio{ 0 };
	sdl_audio.format = map_audio_formot[avf->format];
	sdl_audio.channels = avf->ch_layout.nb_channels;
	sdl_audio.samples = avf->linesize[0] / LearnVideo::sample_bit_size[avf->format] / avf->ch_layout.nb_channels;
	sdl_audio.silence = 0;
	sdl_audio.freq = avf->sample_rate;
	sdl_audio.callback = callback;

	if (SDL_OpenAudio(&sdl_audio, nullptr)) throw "SDL_OpenAudio failed!!!\n";
}