#include"learnffmpeg.h"
#include"LearnSDL.h"

int main(int argc, char* args[])
{
	std::string path;

	if (argc >= 2)
		path = args[1];
	else
		path = "D:\\BaiduNetdiskDownload\\[Nekomoe kissaten][GIRLS BAND CRY][06][1080p][JPSC].mp4";

	LearnVideo LV;
	LearnVideo::RESULT err = LV.open(path.c_str());
	if (err != LearnVideo::SUCCESS) return -1;

    LearnSDL::InitPlayer(LV, "test");
	LearnSDL::StartPlayer();

	system("pause");
	return 0;
}