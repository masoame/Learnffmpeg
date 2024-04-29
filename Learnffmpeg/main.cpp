#include"learnffmpeg.h"
#include"LearnSDL.h"

int main(int argc, char* args[])
{
	std::string path;

	if (argc >= 2)
		path = args[1];
	else
		path = "D:\\test.mp3";

	LearnVideo LV;
	LearnVideo::RESULT err = LV.open(path.c_str());
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