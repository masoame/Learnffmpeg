#include"learnffmpeg.h"

//"D:\BaiduNetdiskDownload\[UHA-WINGS][Oroka na Tenshi wa Akuma to Odoru][05][x264 1080p][CHS].mp4"
int main()
{
	LearnVideo v;
	cv::namedWindow("eyes");
	if (v.open("D:\\BaiduNetdiskDownload\\[DMG&VCB-Studio] Eiga Daisuki Pompo-san [Ma10p_1080p]\\[DMG&VCB-Studio] Eiga Daisuki Pompo-san [Ma10p_1080p][x265_flac_aac].mkv"))
		if (v.init_decode())
			v.start_video_decode(
				[&v](AVFrame* avf)->bool
				{
					if (v.init_sws(avf, AV_PIX_FMT_BGR24))
					{
						Channel_Planes ch{ {new uint8_t[avf->width * avf->height * 3],avf->width * 3} };
						v.start_sws(avf,&ch);
						cv::Mat mat(avf->height, avf->width, CV_8UC3, ch._data->get());
						cv::imshow("eyes", mat);
						cv::waitKey(20);
					}
					return true;
				},
				[](AVFrame* avf)->bool
				{
					//waveOutBreakLoop()
					return true;
				});
	system("pause");

}
