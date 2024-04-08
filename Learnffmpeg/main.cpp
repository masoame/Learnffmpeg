#include"learnffmpeg.h"
#include"learnopengl.h"

int main(int argc, char* args[])
{
	LearnVideo LV;
	LearnVideo::RESULT err = LV.open("D:\\BaiduNetdiskDownload\\[KitaujiSub] Shuumatsu Train Doko e Iku [02][WebRip][HEVC_AAC][CHT].mp4");
	if (err != LearnVideo::SUCCESS) return -1;
	err = LV.init_decode();
	if (err != LearnVideo::SUCCESS) return -1;

	LV.start_video_decode();


	return 0;
}