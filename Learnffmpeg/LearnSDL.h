#pragma once
#include"common.hpp"
#include"learnffmpeg.h"
namespace LearnSDL
{
	extern LearnVideo* target;
	extern AutoAVFramePtr avf;

	extern Uint8* audio_buf;
	extern Uint8* audio_pos;
	extern int buflen;

	extern bool is_planner;
	extern SDL_AudioFormat map_formot[13];

	inline void bind(LearnVideo* in) noexcept { target = in; }

	extern void SDLCALL default_callback(void* userdata, Uint8* stream, int len);
	extern void InitAudio(SDL_AudioCallback callback = default_callback);
	extern bool flush_buf();
	extern bool format_frame();
};