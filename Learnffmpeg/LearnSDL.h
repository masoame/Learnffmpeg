#pragma once
#include"common.hpp"
#include"learnffmpeg.h"
class LearnSDL
{
public:
	static inline void bind(LearnVideo* LV) { _LV = LV; }
	static void SDLCALL default_callback(void* userdata, Uint8* stream, int len);
	static void InitAudio( SDL_AudioCallback callback = default_callback);
private:	
	static bool flush_buf();
	static LearnVideo* _LV;
	static AutoAVFramePtr avf;
	static Uint8* audio_pos;
	static long long buflen;
};