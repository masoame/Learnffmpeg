#include"learnffmpeg.h"
#include"learnopengl.h"


//int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPreInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
//{
//    LearnOpengl test("dfdf");
//    test.GLFWStartWindow();
//    return 0;
//}

int main(int argc, char* args[])
{




	LearnVideo a;
	if (a.open("D:\\BaiduNetdiskDownload\\[ANi]  - 15 [1080P][Baha][WEB-DL][AAC AVC][CHT].mp4"))
		if (a.init_decode())
			a.start_video_decode(
				[](AVFrame* avf)->bool
				{
					
					return true;
				},
				[](AVFrame* avf)->bool
				{
					std::cout << "audio rate: " << avf->sample_rate << std::endl;
					std::cout << "nb_channels: " << avf->ch_layout.nb_channels << std::endl;
					system("pause");
					return true;
				}
	);

	return 0;


}