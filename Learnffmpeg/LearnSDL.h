#pragma once
#include"common.h"

class LearnSDL
{
public:
	static void SDLCALL default_callback(void* userdata, Uint8* stream, int len);
	static void InitAudio(AVFrame* avf, SDL_AudioCallback callback);
private:
	static bool isInitAudio;



};