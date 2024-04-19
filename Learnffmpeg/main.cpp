#include"learnffmpeg.h"
#include"learnopengl.h"

int main(int argc, char* args[])
{
	LearnVideo LV;
	LearnVideo::RESULT err = LV.open("D:\\test.mp3");
	if (err != LearnVideo::SUCCESS) return -1;
	err = LV.init_decode();
	if (err != LearnVideo::SUCCESS) return -1;
	
	LV.start_decode_thread();
	
	while (!LV.QueueSize[AVMEDIA_TYPE_AUDIO]) { Sleep(10); };

	AVFrame* avf;

	LV.FrameQueue[AVMEDIA_TYPE_AUDIO].try_pop(avf);

	std::cout << avf->format << std::endl;
	std::cout << avf->ch_layout.nb_channels << std::endl;
	std::cout << avf->nb_samples << std::endl;
	std::cout << avf->sample_rate << std::endl;


	system("pause");
	return 0;
}