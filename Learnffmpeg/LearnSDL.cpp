#include "LearnSDL.h"

namespace LearnSDL
{

	SDL_AudioFormat map_audio_formot[13]{
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

	Uint8* audio_buf = new Uint8[sample_buf_size];

	LearnVideo* target = nullptr;
	Uint8* audio_pos = nullptr;
	int buflen = 0;

	AutoAVFramePtr avf = nullptr;
	bool is_planner = false;
	bool flush_buf()
	{
		avf.reset();

		while (target->FrameQueue[AVMEDIA_TYPE_AUDIO].empty()) Sleep(1);
		target->FrameQueue[AVMEDIA_TYPE_AUDIO].try_pop(avf);
		target->QueueSize[AVMEDIA_TYPE_AUDIO]--;

		if (avf == nullptr)return false;
		return true;
	}

	bool format_frame()
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

	void SDLCALL default_callback(void* userdata, Uint8* stream, int len)
	{
		//Çå¿ÕÁ÷
		SDL_memset(stream, 0, len);
		if (buflen == 0)
		{
			if (flush_buf()) format_frame(); 
			else { SDL_CloseAudio(); return; }
		}

		len = buflen > len ? len : buflen;
		memcpy(stream, audio_pos, len);
		audio_pos += len;
		buflen -= len;

		return;
	}

	void InitAudio(SDL_AudioCallback callback)
	{
		if(SDL_Init(SDL_INIT_AUDIO))throw "SDL_init error";

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
		sdl_audio.samples = avf->linesize[0] / LearnVideo::sample_bit_size[avf->format]/ avf->ch_layout.nb_channels;
		sdl_audio.silence = 0;
		sdl_audio.freq = avf->sample_rate;
		sdl_audio.callback = callback;

 		if (SDL_OpenAudio(&sdl_audio, nullptr)) throw "SDL_OpenAudio failed!!!\n";
	}
}