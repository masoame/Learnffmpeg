#include"learnffmpeg.h"

//"D:\BaiduNetdiskDownload\[UHA-WINGS][Oroka na Tenshi wa Akuma to Odoru][05][x264 1080p][CHS].mp4"
int main()
{
	LearnVideo v;
	if (v.open("D:\\BaiduNetdiskDownload\\[ANi]  - 15 [1080P][Baha][WEB-DL][AAC AVC][CHT].mp4"))
		if (v.init_decode())
			v.close();
	system("pause");

}
