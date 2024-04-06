#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS

//ffmpeg
extern"C"
{
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libswscale/swscale.h>
#include<libavfilter/avfilter.h>
#include<libavutil/avutil.h>
#include<libswresample/swresample.h>
#include <libavutil/channel_layout.h>
}
//opencv
#include<opencv2/opencv.hpp>

//opengl
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

//c plus plus lib
#include<iostream>
#include<functional>
#include<initializer_list>
#include<type_traits>

//c lib
#include<WinSock2.h>
#include<windows.h>

//SDL
extern"C"
{
#include<SDL.h>
#include<SDL_main.h>
}

#pragma comment(lib,"Ws2_32.lib")

//����RAII�ڴ����
template <auto F>
using Functor = std::integral_constant<std::remove_reference_t<decltype(F)>, F>;

template<class T = void, class DeleteFunction = Functor<CloseHandle>, bool isSecPtr = false>
struct AutoPtr
{
	AutoPtr() noexcept : ptr(nullptr) {}
	explicit AutoPtr(const AutoPtr& Autoptr) = delete;
	explicit AutoPtr(AutoPtr&& Autoptr) noexcept : ptr(Autoptr.release()) { std::cout << "&&" << std::endl; }
	AutoPtr(T* _ptr) noexcept : ptr(_ptr) {}

	void operator=(T* _ptr) { this->ptr.reset(_ptr); }

	operator T* () const { return this->ptr.get(); }
	operator bool() const { return this->ptr.get() != nullptr; }

	T** operator&() { static_assert(sizeof(*this) == sizeof(void*)); assert(ptr); return (T**)this; }
	T* operator->() { return this->ptr.get(); }

	T* release() { return ptr.release(); }
private:
	struct DeletePrimaryPtr { void operator()(void* _ptr) { DeleteFunction()(static_cast<T*>(_ptr)); } };
	struct DeleteSecPtr { void operator()(void* _ptr) { DeleteFunction()(reinterpret_cast<T**>(this)); } };
	using DeletePtr = std::conditional<isSecPtr, DeleteSecPtr, DeletePrimaryPtr>::type;
	std::unique_ptr<T, DeletePtr> ptr;
};

using AutoAVPacketPtr = AutoPtr<AVPacket, Functor<av_packet_free>, true>;
using AutoAVCodecContextPtr = AutoPtr<AVCodecContext, Functor<avcodec_free_context>, true>;
using AutoAVFormatContextPtr = AutoPtr<AVFormatContext, Functor<avformat_free_context>, false>;
using AutoSwsContextPtr = AutoPtr<SwsContext, Functor<sws_freeContext>, false>;
using AutoSwrContextPtr = AutoPtr<SwrContext, Functor<swr_free>, true>;

using AutoAVFramePtr = AutoPtr<AVFrame, Functor<av_frame_free>, true>;
template<>
inline AutoAVFramePtr::AutoPtr(AutoAVFramePtr&& Autoptr) noexcept : ptr(Autoptr.release()) { Autoptr = av_frame_alloc(); }