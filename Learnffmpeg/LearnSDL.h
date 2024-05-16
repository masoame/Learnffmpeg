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
	//�������С
	static unsigned char bit_size;
	//���������ݴ�С =����������) nb_sample * (�������С) bit_size
	static int audio_buflen;
	//ԭʼ֡�Ƿ�Ϊplanner��ʽ
	static bool is_planner;

	//��Ƶ���
	static SDL_Window* sdl_win;
	//��Ⱦ��
	static SDL_Renderer* sdl_renderer;
	//����
	static SDL_Texture* sdl_texture;
	//֡������
	static char* video_buf;
	//
	static SDL_Rect rect;
	//------------------------------------------------------------------
public:
	//��ʼ�����ŭh��
	static void InitPlayer(LearnVideo& rely, const char* WindowName, SDL_AudioCallback callback = default_callback);
	static void StartPlayer();

private:
	//̎����֡
	static void convert_video_frame(AVFrame* work,char* &buf) noexcept;
	//̎�����l��
	static void convert_audio_frame(AVFrame* work, char*& buf) noexcept;
	//��ʼ����Ƶ���Ż���
	static void InitAudio(SDL_AudioCallback callback);
	//��ʼ����Ƶ����
	static void InitVideo(const char* title);
	//Ĭ�����l�ص�����
	static void SDLCALL default_callback(void* userdata, Uint8* stream, int len) noexcept;
	//��֡�����ز���֡
	static bool format_frame() noexcept;
};