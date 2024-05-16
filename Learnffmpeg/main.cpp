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
	err = LV.init_decode();

    LV.insert_callback[AVMEDIA_TYPE_VIDEO] = LearnSDL::convert_frame;


	if (err != LearnVideo::SUCCESS) return -1;
	LV.start_decode_thread();

	LearnSDL::bind(&LV);
	LearnSDL::InitAudio();
	LearnSDL::InitVideo("test");

    auto& work = LV.avframe_work[AVMEDIA_TYPE_VIDEO];

    Sleep(1000);


    SDL_PauseAudio(0);
    while (work.first != nullptr)
	{
        if (SDL_UpdateTexture(LearnSDL::sdl_texture, NULL,work.second, work.first->width))
        {
            std::cout << SDL_GetError() << std::endl;
            return -1;
        }

        // 6. ������Ļ
        if (SDL_RenderClear(LearnSDL::sdl_renderer))
        {
            std::cout << SDL_GetError() << std::endl;
            return -1;
        }

        // 7. ���Ʋ��ʵ���Ⱦ������
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = work.first->width;
        rect.h = work.first->height;
        if (SDL_RenderCopy(LearnSDL::sdl_renderer, LearnSDL::sdl_texture, NULL, &rect))
        {
            std::cout << SDL_GetError() << std::endl;
            return -1;
        }

        // 8. ִ����Ⱦ����
        SDL_RenderPresent(LearnSDL::sdl_renderer);
        LV.flush_frame(AVMEDIA_TYPE_VIDEO);
        Sleep(30);
	}
	system("pause");
	return 0;
}