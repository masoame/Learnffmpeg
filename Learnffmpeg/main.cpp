#include"learnffmpeg.h"
#include"LearnSDL.h"

int main(int argc, char* args[])
{
	LearnVideo LV;
	LearnVideo::RESULT err = LV.open("D:\\test.mp3");
	if (err != LearnVideo::SUCCESS) return -1;
	err = LV.init_decode();
	if (err != LearnVideo::SUCCESS) return -1;
	
	LV.start_decode_thread();
	
	//≥ı ºªØ
	LearnSDL::bind(&LV);
	LearnSDL::InitAudio();
	system("pause");
	SDL_PauseAudio(0);
	system("pause");
	return 0;
}