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


template <auto F>
using Functor = std::integral_constant<std::remove_reference_t<decltype(F)>, F>;

template<class T, class DeleteFunction, bool isSecPtr = false>
struct AutoPtr
{
	AutoPtr() noexcept: ptr(nullptr) {}
	AutoPtr(T* _ptr) noexcept : ptr(_ptr) {}
	void operator=(T* _ptr) { this->ptr.reset(_ptr); }
	operator T*() { return this->ptr.get(); }
	T** operator&() { static_assert(sizeof(*this) == sizeof(void*)); assert(ptr); return (T**)this; }
	T* operator->() { return this->ptr.get(); }
	operator bool() { return this->ptr.get() != nullptr; }
private:
	struct DeletePrimaryPtr { void operator()(void* _ptr) { DeleteFunction()(static_cast<T*>(_ptr)); } };
	struct DeleteSecPtr { void operator()(void* _ptr) { DeleteFunction()(reinterpret_cast<T**>(this)); } };
	using DeletePtr = std::conditional<isSecPtr, DeleteSecPtr, DeletePrimaryPtr>::type;
	std::unique_ptr<T, DeletePtr> ptr;
};

using AutoAVPacketPtr = AutoPtr<AVPacket, Functor<av_packet_free>, true>;
using AutoAVFramePtr = AutoPtr<AVFrame, Functor<av_frame_free>, true>;
using AutoAVCodecContextPtr = AutoPtr<AVCodecContext, Functor<avcodec_free_context>, true>;
using AutoAVFormatContextPtr = AutoPtr<AVFormatContext, Functor<avformat_free_context>, false>;


