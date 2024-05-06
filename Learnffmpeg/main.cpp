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
	if (err != LearnVideo::SUCCESS) return -1;
	LV.start_decode_thread();

	LearnSDL::bind(&LV);
	LearnSDL::InitAudio();
	LearnSDL::InitVideo("test");

    LearnVideo::AutoAVFramePtr& work = LV.avframe_work[AVMEDIA_TYPE_VIDEO];
    char* buf = new char[work->width * work->height * 3 * 2];

    Sleep(1000);


    SDL_PauseAudio(0);
	while (work!=nullptr)
	{

        char* temp = buf;
        memcpy(buf, work->data[0], work->linesize[0] * work->height);
        buf += work->linesize[0] * work->height;
        memcpy(buf, work->data[1], work->linesize[1] * work->height / 2);
        buf += work->linesize[1] * work->height/2;
        memcpy(buf, work->data[2], work->linesize[2] * work->height / 2);
        buf=temp;
  
        if (SDL_UpdateTexture(LearnSDL::sdl_texture, NULL, buf, work->width))
        {
            std::cout << SDL_GetError() << std::endl;
            return -1;
        }

        // 6. 清理屏幕
        if (SDL_RenderClear(LearnSDL::sdl_renderer))
        {
            std::cout << SDL_GetError() << std::endl;
            return -1;
        }

        // 7. 复制材质到渲染器对象
        SDL_Rect rect;
        rect.x = 0;
        rect.y = 0;
        rect.w = work->width;
        rect.h = work->height;
        if (SDL_RenderCopy(LearnSDL::sdl_renderer, LearnSDL::sdl_texture, NULL, &rect))
        {
            std::cout << SDL_GetError() << std::endl;
            return -1;
        }

        // 8. 执行渲染操作
        SDL_RenderPresent(LearnSDL::sdl_renderer);
        LV.flush_frame(AVMEDIA_TYPE_VIDEO);
        Sleep(30);
	}

	
	system("pause");
	return 0;
}