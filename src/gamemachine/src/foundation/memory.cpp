#include "stdafx.h"
#include "memory.h"
#include "debug.h"
#include <gmthread.h>

GMint32 gm_s_numAlignedAllocs = 0;
GMint32 gm_s_numAlignedFree = 0;
GMint32 gm_s_totalBytesAlignedAllocs = 0;//detect memory leaks

static void *gmAllocDefault(size_t size)
{
	return malloc(size);
}

static void gmFreeDefault(void *ptr)
{
	free(ptr);
}

static gmAllocFunc* gm_s_allocFunc = gmAllocDefault;
static gmFreeFunc* gm_s_freeFunc = gmFreeDefault;

template <typename T>T* gmAlignPointer(T* unalignedPtr, size_t alignment)
{
	struct GMConvertPointerSizeT
	{
		union
		{
			T* ptr;
			size_t integer;
		};
	};
	GMConvertPointerSizeT converter;
	const size_t bit_mask = ~(alignment - 1);
	converter.ptr = unalignedPtr;
	converter.integer += alignment - 1;
	converter.integer &= bit_mask;
	return converter.ptr;
}

static inline void *gmAlignedAllocDefault(size_t size, GMint32 alignment)
{
	void *ret;
	char *real;
	real = (char *)gm_s_allocFunc(size + sizeof(void *) + (alignment - 1));
	if (real) {
		ret = gmAlignPointer(real + sizeof(void *), alignment);
		*((void **)(ret)-1) = (void *)(real);
	}
	else {
		ret = (void *)(real);
	}
	return (ret);
}

static inline void gmAlignedFreeDefault(void *ptr)
{
	void* real;

	if (ptr) {
		real = *((void **)(ptr)-1);
		gm_s_freeFunc(real);
	}
}

static gmAlignedAllocFunc* gm_s_alignedAllocFunc = gmAlignedAllocDefault;
static gmAlignedFreeFunc* gm_s_alignedFreeFunc = gmAlignedFreeDefault;

void gmAlignedAllocSetCustomAligned(gmAlignedAllocFunc *allocFunc, gmAlignedFreeFunc *freeFunc)
{
	gm_s_alignedAllocFunc = allocFunc ? allocFunc : gmAlignedAllocDefault;
	gm_s_alignedFreeFunc = freeFunc ? freeFunc : gmAlignedFreeDefault;
}

void gmAlignedAllocSetCustom(gmAllocFunc *allocFunc, gmFreeFunc *freeFunc)
{
	gm_s_allocFunc = allocFunc ? allocFunc : gmAllocDefault;
	gm_s_freeFunc = freeFunc ? freeFunc : gmFreeDefault;
}

void* AlignedMemoryAlloc::gmAlignedAllocInternal(size_t size, GMint32 alignment)
{
	gm_s_numAlignedAllocs++;
	void* ptr;
	ptr = gm_s_alignedAllocFunc(size, alignment);
	return ptr;
}

void AlignedMemoryAlloc::gmAlignedFreeInternal(void* ptr)
{
	if (!ptr)
		return;

	gm_s_numAlignedFree++;
	gm_s_alignedFreeFunc(ptr);
}
