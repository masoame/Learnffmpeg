#pragma once
#include"common.h"

class LearnSDL
{
public:
	
	static void InitAudio(AVFrame* avf, SDL_AudioCallback callback);
private:
	static bool isInitAudio;
};