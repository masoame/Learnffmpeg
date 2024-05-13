#pragma once
#include"common.hpp"
#include"learnffmpeg.h"
namespace LearnSDL
{
	using AutoWindowPtr = AutoPtr<SDL_Window, Functor<SDL_DestroyWindow>, false>;
	using AutoRendererPtr = AutoPtr<SDL_Window, Functor<SDL_DestroyRenderer>, false>;
	using AutoTexturePtr = AutoPtr<SDL_Window, Functor<SDL_DestroyTexture>, false>;

	//�洢�������ڴ��С
	constexpr const int sample_buf_size = 1024 * 1024 * 8;

	//ffmpeg format to SDL format
	extern SDL_AudioFormat const map_audio_formot[13];
	extern const std::map<AVPixelFormat, SDL_PixelFormatEnum> map_video_format;


	//��Ƶ����
	extern LearnVideo* target;

	//--------------------------------------------------------------

	//������
	extern Uint8* audio_buf;
	extern Uint8* audio_pos;
	//�������С
	extern unsigned char bit_size;
	//���������ݴ�С =����������) nb_sample * (�������С) bit_size
	extern int audio_buflen;
	//ԭʼ֡�Ƿ�Ϊplanner��ʽ
	extern bool is_planner;

	//��Ƶ���
	extern SDL_Window* sdl_win;
	//��Ⱦ��
	extern SDL_Renderer* sdl_renderer;
	//����
	extern SDL_Texture* sdl_texture;
	//֡������
	extern char* video_buf;



	//------------------------------------------------------------------

	//����Ƶ����
	inline void bind(LearnVideo* in) noexcept { target = in; }

	//Ĭ�ϻص�����
	extern void SDLCALL default_callback(void* userdata, Uint8* stream, int len) noexcept;
	//��ʼ����Ƶ���Ż���
	extern void InitAudio(SDL_AudioCallback callback = default_callback);

	//��ʼ����Ƶ����
	extern void InitVideo(const char* title);

	//ת��֡
	char* convert_frame(AVFrame* work) noexcept;

	//��֡�����ز���֡
	extern bool format_frame() noexcept;
};