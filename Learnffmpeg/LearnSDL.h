#pragma once
#include"common.hpp"
#include"learnffmpeg.h"
class LearnSDL
{
	constexpr static const int sample_buf_size = 1024 * 1024 * 8;

	using AutoWindowPtr = AutoPtr<SDL_Window, Functor<SDL_DestroyWindow>, false>;
	using AutoRendererPtr = AutoPtr<SDL_Window, Functor<SDL_DestroyRenderer>, false>;
	using AutoTexturePtr = AutoPtr<SDL_Window, Functor<SDL_DestroyTexture>, false>;

	static SDL_AudioFormat const map_audio_formot[13];
	static const std::map<AVPixelFormat, SDL_PixelFormatEnum> map_video_format;

	static LearnVideo* target;

	static Uint8* audio_buf;
	static Uint8* audio_pos;
	//采样点大小
	static unsigned char bit_size;
	//采样到数据大小 =（采样个数) nb_sample * (采样点大小) bit_size
	static int audio_buflen;
	//原始帧是否为planner格式
	static bool is_planner;

	//视频句柄
	static SDL_Window* sdl_win;
	//渲染器
	static SDL_Renderer* sdl_renderer;
	//纹理
	static SDL_Texture* sdl_texture;
	//帧缓冲区
	static char* video_buf;
	//
	static SDL_Rect rect;
	//------------------------------------------------------------------
public:
	//初始化播放環境
	static void InitPlayer(LearnVideo& rely, const char* WindowName, SDL_AudioCallback callback = default_callback);
	static void StartPlayer();

private:
	//處理畫面帧
	static void convert_video_frame(AVFrame* work,char* &buf) noexcept;
	//處理音頻幀
	static void convert_audio_frame(AVFrame* work, char*& buf) noexcept;
	//初始化音频播放环境
	static void InitAudio(SDL_AudioCallback callback);
	//初始化视频环境
	static void InitVideo(const char* title);
	//默认音頻回调函数
	static void SDLCALL default_callback(void* userdata, Uint8* stream, int len) noexcept;
	//对帧进行重采样帧
	static bool format_frame() noexcept;
};