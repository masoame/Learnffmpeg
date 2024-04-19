#include "LearnSDL.h"

bool LearnSDL::isInitAudio = false;

static Uint8* audio_buf = NULL;
static Uint8* audio_pos = NULL;
static size_t buffer_len = 0;

void SDLCALL LearnSDL::default_callback(void* userdata, Uint8* stream, int len)
{
	memset(stream, 0, len);
	if (buffer_len == 0) return;

	len = (len < buffer_len) ? len : buffer_len;
	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);

	audio_pos += len;
	buffer_len -= len;

	return;
}

void LearnSDL::InitAudio(AVFrame* avf, SDL_AudioCallback callback)
{
	if (isInitAudio)return;

	SDL_Init(SDL_INIT_AUDIO);
	SDL_AudioSpec sdl_audio{ 0 };
	sdl_audio.format = (AVSampleFormat)avf->format;
	sdl_audio.channels = avf->ch_layout.nb_channels;
	sdl_audio.samples = avf->nb_samples;
	sdl_audio.silence = 0;
	sdl_audio.freq = avf->sample_rate;
	sdl_audio.callback = callback;

	if (SDL_OpenAudio(&sdl_audio, nullptr))
	{
		std::cout << "audio open failed !!!!" << std::endl;
	}


	SDL_PauseAudio(0);

	do {

		audio_pos = audio_buf;

		while (audio_pos < (audio_buf + buffer_len)) {
			SDL_Delay(1);
		}

	} while (buffer_len != 0);
}