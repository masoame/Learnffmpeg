#pragma once
#include"common.hpp"
#include"learnffmpeg.h"
namespace LearnSDL
{
	//ffmpeg format to SDL format
	extern SDL_AudioFormat map_audio_formot[13];
	//映射对应编码一个采样点的大小
	extern unsigned char sample_bit_size[13];

	//音频对象
	extern LearnVideo* target;
	//工作帧
	extern AutoAVFramePtr avf;

	//缓存区
	extern Uint8* audio_buf;
	//工作指针
	extern Uint8* audio_pos;
	//采样点大小
	extern unsigned char bit_size;
	//采样到数据大小 =（采样个数) nb_sample * (采样点大小) bit_size
	extern int buflen;
	//原始帧是否为planner格式
	extern bool is_planner;

	//绑定音频对象
	inline void bind(LearnVideo* in) noexcept { target = in; }

	//默认回调函数
	extern void SDLCALL default_callback(void* userdata, Uint8* stream, int len);
	//初始化音频播放环境
	extern void InitAudio(SDL_AudioCallback callback = default_callback);
	//刷新帧
	extern bool flush_buf();
	//对帧进行重采样帧
	extern bool format_frame();
};