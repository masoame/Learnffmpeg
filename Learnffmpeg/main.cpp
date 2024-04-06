#include"learnffmpeg.h"
#include"learnopengl.h"

int main(int argc, char* args[])
{
	LearnVideo LV;
	LearnVideo::RESULT err = LV.open("D:\\BaiduNetdiskDownload\\[ANi]  - 15 [1080P][Baha][WEB-DL][AAC AVC][CHT].mp4");
	if (err != LearnVideo::SUCCESS) return -1;
	err = LV.init_decode();
	if (err != LearnVideo::SUCCESS) return -1;
	LV.start_video_decode();

	return 0;
}