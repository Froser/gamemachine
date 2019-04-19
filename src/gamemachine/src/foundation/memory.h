#ifndef __GM_MEMORY_H__
#define __GM_MEMORY_H__
#include "defines.h"
BEGIN_NS

typedef void *(gmAlignedAllocFunc)(size_t size, GMint32 alignment);
typedef void (gmAlignedFreeFunc)(void *memblock);
typedef void *(gmAllocFunc)(size_t size);
typedef void (gmFreeFunc)(void *memblock);
void gmAlignedAllocSetCustom(gmAllocFunc *allocFunc, gmFreeFunc *freeFunc);
void gmAlignedAllocSetCustomAligned(gmAlignedAllocFunc *allocFunc, gmAlignedFreeFunc *freeFunc);

#define gmAlignedAlloc(size,alignment) gm::AlignedMemoryAlloc::gmAlignedAllocInternal(size,alignment)
#define gmAlignedFree(ptr) gm::AlignedMemoryAlloc::gmAlignedFreeInternal(ptr)

class GM_EXPORT AlignedMemoryAlloc
{
public:
	static void* gmAlignedAllocInternal(size_t size, GMint32 alignment);
	static void gmAlignedFreeInternal(void* ptr);
};

template < typename T, unsigned Alignment = 16>
class AlignedAllocator
{
	typedef AlignedAllocator< T, Alignment > my_type;

public:
	AlignedAllocator() = default;

public:
	template < typename Other >
	AlignedAllocator(const AlignedAllocator< Other, Alignment > &) {}

	typedef const T* const_pointer;
	typedef const T& const_reference;
	typedef T* pointer;
	typedef T& reference;
	typedef T value_type;

	pointer address(reference ref) const { return &ref; }
	const_pointer address(const_reference  ref) const { return &ref; }
	pointer allocate(GMsize_t n, const_pointer * hint = 0) {
		(void)hint;
		return reinterpret_cast<pointer>(gmAlignedAlloc(sizeof(value_type) * n, Alignment));
	}
	void construct(pointer ptr, const value_type & value) { new (ptr) value_type(value); }
	void deallocate(pointer ptr, GMsize_t)
	{
		gmAlignedFree(reinterpret_cast<void *>(ptr));
	}
	void destroy(pointer ptr) { ptr->~value_type(); }


	template < typename O > struct rebind {
		typedef AlignedAllocator< O, Alignment > other;
	};
	template < typename O >
	my_type & operator=(const AlignedAllocator< O, Alignment > &) { return *this; }

	friend bool operator==(const my_type &, const my_type &) { return true; }
	friend bool operator!=(const my_type &, const my_type &) { return false; }
};

// override new, delete，对齐分配内存
#define GM_DECLARE_ALIGNED_ALLOCATOR() \
	public:   \
	inline void* operator new(size_t sizeInBytes){ return gmAlignedAlloc(sizeInBytes, 16); }   \
	inline void  operator delete(void* ptr)         { gmAlignedFree(ptr); }   \
	inline void* operator new(size_t, void* ptr){ return ptr; }   \
	inline void  operator delete(void*, void*)      { }   \
	inline void* operator new[](size_t sizeInBytes)   { return gmAlignedAlloc(sizeInBytes, 16); }   \
	inline void  operator delete[](void* ptr)         { gmAlignedFree(ptr); }   \
	inline void* operator new[](size_t, void* ptr)   { return ptr; }   \
	inline void  operator delete[](void*, void*)      {}

class GMAlignmentObject
{
	GM_DECLARE_ALIGNED_ALLOCATOR();
};

#define GM_ALIGNED_STRUCT_FROM(name, from) GM_ALIGNED_16(struct) name : public from
#define GM_ALIGNED_STRUCT(name) GM_ALIGNED_STRUCT_FROM(name, gm::GMAlignmentObject)

#if GM_WINDOWS
template <typename T>
using AlignedVector = Vector<T, AlignedAllocator<T>>;
#else
template <typename T>
using AlignedVector = Vector<T>;
#endif

END_NS
#endif