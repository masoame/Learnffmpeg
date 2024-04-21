#include "LearnSDL.h"

LearnVideo* LearnSDL::_LV;
Uint8* LearnSDL::audio_pos;
long long LearnSDL::buflen;
AutoAVFramePtr LearnSDL::avf;

bool LearnSDL::flush_buf()
{
	while (!_LV->QueueSize[AVMEDIA_TYPE_AUDIO]) { Sleep(1); };
	
	_LV->FrameQueue[AVMEDIA_TYPE_AUDIO].try_pop(avf);
	_LV->QueueSize[AVMEDIA_TYPE_AUDIO]--;
	audio_pos = avf->data[0];
	buflen = avf->linesize[0];

	return true;
}

void SDLCALL LearnSDL::default_callback(void* userdata, Uint8* stream, int len)
{
	SDL_memset(stream, 0, len);
	if (buflen == 0 && !flush_buf()) 
	{
		SDL_CloseAudio(); 
		return;
	}

	len = buflen > len ? len : buflen;
	memcpy(stream, audio_pos, len);
	audio_pos += len;
	buflen -= len;
	
	if (buflen == 0) 
		avf.reset(nullptr);
	


	return;
}

void LearnSDL::InitAudio(SDL_AudioCallback callback)
{
	SDL_Init(SDL_INIT_AUDIO);

	flush_buf();

	SDL_AudioSpec sdl_audio{ 0 };
	sdl_audio.format = AUDIO_F32SYS;
	sdl_audio.channels = 1;
	sdl_audio.samples = avf->nb_samples;
	sdl_audio.silence = 0;
	sdl_audio.freq = avf->sample_rate;
	sdl_audio.callback = callback;


	if (SDL_OpenAudio(&sdl_audio, nullptr))
	{
		std::cout << "audio open failed !!!!" << std::endl;
	}

	if (av_sample_fmt_is_planar((AVSampleFormat)avf->format)) {};

	audio_pos = avf->data[0];
	buflen = avf->linesize[0];

}