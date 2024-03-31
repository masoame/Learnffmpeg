#include "LearnSDL.h"

bool LearnSDL::isInitAudio = false;


void LearnSDL::InitAudio(AVFrame* avf, SDL_AudioCallback callback)
{
	if (isInitAudio)return;

	SDL_Init(SDL_INIT_AUDIO);
	SDL_AudioSpec sdl_audio{ 0 };
	sdl_audio.channels = avf->ch_layout.nb_channels;
	sdl_audio.samples = avf->nb_samples;
	sdl_audio.freq = avf->sample_rate;
	sdl_audio.callback = callback;

	if (SDL_OpenAudio(&sdl_audio, nullptr))
	{
		std::cout << "audio open fail !!!!" << std::endl;
	}


}
