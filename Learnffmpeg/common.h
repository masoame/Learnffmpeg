#pragma once
#define _CRT_SECURE_NO_WARNINGS
//ffmpeg
extern"C"
{
#include<libavformat/avformat.h>
#include<libavcodec/avcodec.h>
#include<libswscale/swscale.h>
#include<libavfilter/avfilter.h>
#include<libavutil/avutil.h>
}
//opencv
#include<opencv2/opencv.hpp>

//c plus plus lib
#include<iostream>
#include<functional>

//c lib
#include<windows.h>

struct PermissivePointer
{
	template <typename T>
	operator T* () { return (T*)p; }
	void* p;
};

template <auto F>
using Functor = std::integral_constant<std::remove_reference_t<decltype(F)>, F>;

template<class T, class DeleteFunction>
struct AutoPtr
{
	AutoPtr() : ptr(nullptr) {}
	AutoPtr(T* _ptr) : ptr(_ptr) {}
	void operator=(T* _ptr) { this->ptr.reset(_ptr); }
	operator T*() { return ptr.get(); }
	T** operator&() { static_assert(sizeof(*this) == sizeof(void*)); assert(ptr); return (T**)this; }
	T* operator->() { return ptr.get(); }
	operator bool() { return ptr.get() != nullptr; }
private:
	struct DeletePtr { void operator()(void* _ptr) { DeleteFunction()(PermissivePointer{ this }); } };
	std::unique_ptr<T, DeletePtr> ptr;
};

using AutoAVPacketPtr = AutoPtr<AVPacket, Functor<av_packet_free>>;
using AutoAVFramePtr = AutoPtr<AVFrame, Functor<av_frame_free>>;
using AutoAVCodecContextPtr = AutoPtr<AVCodecContext, Functor<avcodec_free_context>>;


