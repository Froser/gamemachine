#ifndef __GMOBJECT_H__
#define __GMOBJECT_H__
#include "defines.h"
#include "memory.h"

// GameMachine采用数据和方法分离的方式，可以为一个类定义一个私有结构存储数据
#define DEFINE_PRIVATE(className) \
	public: \
	typedef className##Private Data; \
	private: \
	GM_ALIGNED_16 className##Private m_data; \
	protected: \
	className##Private& data() { return m_data; }
#define D(d) auto& d = data()
#define DEFINE_PRIVATE_ON_HEAP(className) \
	private: \
	typedef className##Private DataType; \
	className##Private* m_data; \
	protected: \
	className##Private*& data() { return m_data; }
#define D_BASE(base, d) auto& d = base::data()

// 重在new, delete，对齐分配内存
#if USE_SIMD
#define GM_DECLARE_ALIGNED_ALLOCATOR() \
	public:   \
	inline void* operator new(size_t sizeInBytes)   { return gmAlignedAlloc(sizeInBytes,16); }   \
	inline void  operator delete(void* ptr)         { gmAlignedFree(ptr); }   \
	inline void* operator new(size_t, void* ptr)   { return ptr; }   \
	inline void  operator delete(void*, void*)      { }   \
	inline void* operator new[](size_t sizeInBytes)   { return gmAlignedAlloc(sizeInBytes,16); }   \
	inline void  operator delete[](void* ptr)         { gmAlignedFree(ptr); }   \
	inline void* operator new[](size_t, void* ptr)   { return ptr; }   \
	inline void  operator delete[](void*, void*)      { }
#else
#define GM_DECLARE_ALIGNED_ALLOCATOR()
#endif

// 为一个对象定义private部分
#define GM_PRIVATE_OBJECT(name) GM_ALIGNED_16_(struct) name##Private : public GMObject

BEGIN_NS
// 所有GM对象的基类，如果可以用SSE指令，那么它是16字节对齐的
GM_ALIGNED_16_(class) GMObject
{
	GM_DECLARE_ALIGNED_ALLOCATOR();

public:
	virtual ~GMObject();
};

END_NS
#endif