#pragma once
#include"common.hpp"
#include"learnffmpeg.h"
namespace LearnSDL
{
	constexpr int sample_buf_size = 1024 * 1024 * 8;

	//ffmpeg format to SDL format
	extern SDL_AudioFormat map_audio_formot[13];

	//��Ƶ����
	extern LearnVideo* target;
	//����֡
	extern AutoAVFramePtr avf;

	//������
	extern Uint8* audio_buf;
	//����ָ��
	extern Uint8* audio_pos;
	//�������С
	extern unsigned char bit_size;
	//���������ݴ�С =����������) nb_sample * (�������С) bit_size
	extern int buflen;
	//ԭʼ֡�Ƿ�Ϊplanner��ʽ
	extern bool is_planner;

	//����Ƶ����
	inline void bind(LearnVideo* in) noexcept { target = in; }

	//Ĭ�ϻص�����
	extern void SDLCALL default_callback(void* userdata, Uint8* stream, int len);
	//��ʼ����Ƶ���Ż���
	extern void InitAudio(SDL_AudioCallback callback = default_callback);
	//ˢ��֡
	extern bool flush_buf();
	//��֡�����ز���֡
	extern bool format_frame();
};