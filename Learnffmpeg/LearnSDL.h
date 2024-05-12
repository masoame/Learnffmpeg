#pragma once
#include"common.hpp"
#include"learnffmpeg.h"
namespace LearnSDL
{
	using AutoWindowPtr = AutoPtr<SDL_Window, Functor<SDL_DestroyWindow>, false>;
	using AutoRendererPtr = AutoPtr<SDL_Window, Functor<SDL_DestroyRenderer>, false>;
	using AutoTexturePtr = AutoPtr<SDL_Window, Functor<SDL_DestroyTexture>, false>;

	//存储采样的内存大小
	constexpr const int sample_buf_size = 1024 * 1024 * 8;

	//ffmpeg format to SDL format
	extern SDL_AudioFormat const map_audio_formot[13];
	extern const std::map<AVPixelFormat, SDL_PixelFormatEnum> map_video_format;


	//音频对象
	extern LearnVideo* target;

	//--------------------------------------------------------------

	//缓存区
	extern Uint8* audio_buf;
	extern Uint8* audio_pos;
	//采样点大小
	extern unsigned char bit_size;
	//采样到数据大小 =（采样个数) nb_sample * (采样点大小) bit_size
	extern int audio_buflen;
	//原始帧是否为planner格式
	extern bool is_planner;

	//视频句柄
	extern SDL_Window* sdl_win;
	//渲染器
	extern SDL_Renderer* sdl_renderer;
	//纹理
	extern SDL_Texture* sdl_texture;
	//帧缓冲区
	extern char* video_buf;



	//------------------------------------------------------------------

	//绑定音频对象
	inline void bind(LearnVideo* in) noexcept { target = in; }

	//默认回调函数
	extern void SDLCALL default_callback(void* userdata, Uint8* stream, int len) noexcept;
	//初始化音频播放环境
	extern void InitAudio(SDL_AudioCallback callback = default_callback);

	//初始化视频环境
	extern void InitVideo(const char* title);

	//转化帧
	char* convert_frame(AVFrame* work) noexcept;

	//对帧进行重采样帧
	extern bool format_frame() noexcept;
};