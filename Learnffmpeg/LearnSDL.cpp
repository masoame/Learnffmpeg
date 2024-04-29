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
bool LearnSDL::is_planner = false;


bool LearnSDL::format_frame() noexcept
{
	AutoAVFramePtr& audio_frame = target->QueueFrame.avframe_work[AVMEDIA_TYPE_AUDIO];

	if (audio_frame == nullptr) return false;
	if (is_planner)
	{
		buflen = sample_buf_size;
		target->sample_planner_to_packed(audio_frame, &audio_buf, &buflen);
		audio_pos = audio_buf;
	}
	else
	{
		audio_pos = audio_frame->data[0];
		buflen = audio_frame->nb_samples;
	}
	return true;
}

void SDLCALL LearnSDL::default_callback(void* userdata, Uint8* stream, int len)noexcept
{
	//Çå¿ÕÁ÷
	SDL_memset(stream, 0, len);
	if (buflen == 0)
	{
		if (target->QueueFrame.flush_frame(AVMEDIA_TYPE_AUDIO) && format_frame()) {}
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
	AutoAVFramePtr& audio_frame = target->QueueFrame.avframe_work[AVMEDIA_TYPE_AUDIO];
	if (SDL_Init(SDL_INIT_AUDIO))throw "SDL_init error";

	if (!target->QueueFrame.flush_frame(AVMEDIA_TYPE_AUDIO))throw "get_frame error";
	if (av_sample_fmt_is_planar((AVSampleFormat)audio_frame->format))
	{
		if (target->init_swr(audio_frame) != LearnVideo::SUCCESS) throw "init_swr() failed";
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