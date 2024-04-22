#include"learnffmpeg.h"
#include"LearnSDL.h"

int main(int argc, char* args[])
{
	if (argc >= 2)
	{
		LearnVideo LV;
		LearnVideo::RESULT err = LV.open(args[1]);
		if (err != LearnVideo::SUCCESS) return -1;
		err = LV.init_decode();
		if (err != LearnVideo::SUCCESS) return -1;
		LV.start_decode_thread();

		LearnSDL::bind(&LV);
		LearnSDL::InitAudio();
		SDL_PauseAudio(0);
		system("pause");
		return 0;
	}

}