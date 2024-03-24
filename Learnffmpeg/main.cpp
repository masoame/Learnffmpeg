#include"learnffmpeg.h"

//"D:\BaiduNetdiskDownload\[UHA-WINGS][Oroka na Tenshi wa Akuma to Odoru][05][x264 1080p][CHS].mp4"
int main()
{
	
	LearnVideo v;

	if(v.open("D:\\BaiduNetdiskDownload\\[ANi]  - 15 [1080P][Baha][WEB-DL][AAC AVC][CHT].mp4"))
		if(v.init_video_decode())
			v.start_video_decode([&](AVFrame* avf)->bool 
				{			
					auto a = LearnVideo::YUV2RGB(avf,"test.bmp",AV_PIX_FMT_BGR24);
					if (!a) return false;
					cv::Mat mat(avf->height ,avf->width , CV_8UC3, a.get());
					cv::namedWindow("eyes", cv::WINDOW_AUTOSIZE);
					cv::imshow("eyes", mat);
					cv::waitKey(30);

					return true; 
				});

}
