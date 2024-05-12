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

template<class T = void, class FreeFunc = Functor<CloseHandle>, bool isSecPtr = false>
struct AutoPtr
{
	/*
	* 构造函数
	* 删除引用构造函数因为包装std::unique_ptr引用构造不符合语义
	*/
	AutoPtr() noexcept {}
	AutoPtr(AutoPtr& Autoptr) = delete;
	explicit AutoPtr(AutoPtr&& Autoptr) noexcept : _ptr(Autoptr.release()) {}
	AutoPtr(T* ptr) noexcept : _ptr(ptr) {}

	/*
	* 重载赋值符号
	*/
	void operator=(T* ptr)noexcept { _ptr.reset(ptr); }
	void operator=(AutoPtr&& Autoptr) noexcept { _ptr.reset(Autoptr.release()); }

	/*
	* 类型转化重载
	*/
	operator const T* () const noexcept { return _ptr.get(); }
	operator T*& () noexcept { return *reinterpret_cast<T**>(this); }
	operator bool() const noexcept { return _ptr.get() != nullptr; }

	/*
	* 运算符重载
	*/
	T** operator&() { static_assert(sizeof(*this) == sizeof(void*)); assert(_ptr); return reinterpret_cast<T**>(this); }
	T* operator->() const noexcept { return _ptr.get(); }

	/*
	* 成员函数
	*/

	void reset(T* ptr = nullptr) noexcept { _ptr.reset(ptr); }
	T* release() noexcept { return _ptr.release(); }
	T* get() const noexcept { return _ptr.get(); }
private:
	struct DeletePrimaryPtr { void operator()(void* ptr) { FreeFunc()(static_cast<T*>(ptr)); } };
	struct DeleteSecPtr { void operator()(void* ptr) { FreeFunc()(reinterpret_cast<T**>(&ptr)); } };
	using DeletePtr = std::conditional<isSecPtr, DeleteSecPtr, DeletePrimaryPtr>::type;
	std::unique_ptr<T, DeletePtr> _ptr;
};


template<class T>
class Circular_Queue
{
	using Type = std::remove_reference<T>::type;


	explicit Circular_Queue(size_t size) :_front(0), _rear(0), _size(size) { static_assert(size >= 2); _Arr.reset(new Type[size]); }

	std::unique_ptr<Type[]> _Arr;
	const size_t _size;

	std::atomic<size_t> _front, _rear;

	void push(const T& target) const { while (_front == ((_rear + 1) % _size)) Sleep(1); _Arr[++_rear % _size] = target; }
	T pop() { while (_rear == _front) Sleep(1); return _Arr[_front++]; }
};


