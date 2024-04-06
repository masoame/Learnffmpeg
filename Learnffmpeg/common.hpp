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
#include<concurrent_queue.h>

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

//定义RAII内存回收
template <auto F>
using Functor = std::integral_constant<std::remove_reference_t<decltype(F)>, F>;

template<class T = void, class DeleteFunction = Functor<CloseHandle>, bool isSecPtr = false>
struct AutoPtr
{
	/*
	* 构造函数
	* 删除引用构造函数因为包装std::unique_ptr引用构造不符合语义
	*/
	AutoPtr() noexcept{}
	explicit AutoPtr(AutoPtr& Autoptr) = delete;
	explicit AutoPtr(AutoPtr&& Autoptr) noexcept : _ptr(Autoptr.release()) {}
	AutoPtr(T* ptr) noexcept : _ptr(ptr) {}

	void operator=(T* ptr)noexcept { _ptr.reset(ptr); }
	void operator=(AutoPtr&& Autoptr) noexcept { _ptr.reset(Autoptr.release()); }

	operator T* () const noexcept { return _ptr.get(); }
	operator bool() const noexcept { return _ptr.get() != nullptr; }

	T** operator&() { static_assert(sizeof(*this) == sizeof(void*)); assert(_ptr); return (T**)this; }
	T* operator->() const noexcept { return _ptr.get(); }

	void reset(T* ptr)noexcept { _ptr.reset(ptr); }
	T* release() noexcept { return _ptr.release(); }
	T* get() const noexcept { return _ptr.get(); }
private:
	struct DeletePrimaryPtr { void operator()(void* ptr) { DeleteFunction()(static_cast<T*>(ptr)); } };
	struct DeleteSecPtr { void operator()(void* ptr) { DeleteFunction()(reinterpret_cast<T**>(this)); } };
	using DeletePtr = std::conditional<isSecPtr, DeleteSecPtr, DeletePrimaryPtr>::type;
	std::unique_ptr<T, DeletePtr> _ptr;
};

using AutoAVPacketPtr = AutoPtr<AVPacket, Functor<av_packet_free>, true>;
using AutoAVCodecContextPtr = AutoPtr<AVCodecContext, Functor<avcodec_free_context>, true>;
using AutoAVFormatContextPtr = AutoPtr<AVFormatContext, Functor<avformat_free_context>, false>;
using AutoSwsContextPtr = AutoPtr<SwsContext, Functor<sws_freeContext>, false>;
using AutoSwrContextPtr = AutoPtr<SwrContext, Functor<swr_free>, true>;

using AutoAVFramePtr = AutoPtr<AVFrame, Functor<av_frame_free>, true>;
template<>
inline AutoAVFramePtr::AutoPtr(AutoAVFramePtr&& Autoptr) noexcept : _ptr(Autoptr.release()) {  }
inline AVFrame* AutoAVFramePtr::release()noexcept { return _ptr.release(); }