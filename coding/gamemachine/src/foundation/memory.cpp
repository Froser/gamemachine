#include "stdafx.h"
#include "memory.h"
#include "debug.h"
#include "gmthreads.h"

GMint gm_s_numAlignedAllocs = 0;
GMint gm_s_numAlignedFree = 0;
GMint gm_s_totalBytesAlignedAllocs = 0;//detect memory leaks

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

static inline void *gmAlignedAllocDefault(size_t size, GMint alignment)
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

#if _DEBUG && GM_DEBUG_MEMORY
// 内存调试
static GMint allocations_id[10241024];
static GMint allocations_bytes[10241024];
static GMint mynumallocs = 0;
static std::string allocations_msg[10241024];

GMint AlignedMemoryAlloc::gmDumpMemoryLeaks()
{
	GMint totalLeak = 0;

	for (GMint i = 0; i < mynumallocs; i++)
	{
		gm_error("[Memory Leak] leaked memory of allocation #%d (%d bytes) %s\n", allocations_id[i], allocations_bytes[i], allocations_msg[allocations_id[i]]);
		totalLeak += allocations_bytes[i];
	}
	if (totalLeak)
	{
		gm_error("[Memory Leak] %d allocations were not freed and leaked together %d bytes\n", mynumallocs, totalLeak);
	}
	return totalLeak;
}

struct GMDebugPtr
{
	union
	{
		void** vptrptr;
		void* vptr;
		GMint* iptr;
		char* cptr;
	};
};

void* AlignedMemoryAlloc::gmAlignedAllocInternal(size_t size, GMint alignment, GMint line, char* filename)
{
	if (size == 0)
	{
		gm_error("Size == 0 error");
		return 0;
	}
	static GMint allocId = 0;

	void *ret;
	char *real;

	gm_s_totalBytesAlignedAllocs += size;
	gm_s_numAlignedAllocs++;

	GMint sz4prt = 4 * sizeof(void *);

	real = (char *)gm_s_allocFunc(size + sz4prt + (alignment - 1));
	if (real) {

		ret = (void*)gmAlignPointer(real + sz4prt, alignment);
		GMDebugPtr p;
		p.vptr = ret;
		p.cptr -= sizeof(void*);
		*p.vptrptr = (void*)real;
		p.cptr -= sizeof(void*);
		*p.iptr = size;
		p.cptr -= sizeof(void*);
		*p.iptr = allocId;

		allocations_id[mynumallocs] = allocId;
		allocations_bytes[mynumallocs] = size;
		allocations_msg[allocId] = filename;
		mynumallocs++;

	}
	else {
		ret = (void *)(real);//??
	}

	allocId++;

	GMint* ptr = (GMint*)ret;
	*ptr = 12;
	return (ret);
}

void AlignedMemoryAlloc::gmAlignedFreeInternal(void* ptr, GMint line, char* filename)
{

	void* real;

	if (ptr)
	{
		gm_s_numAlignedFree++;

		GMDebugPtr p;
		p.vptr = ptr;
		p.cptr -= sizeof(void*);
		real = *p.vptrptr;
		p.cptr -= sizeof(void*);
		GMint size = *p.iptr;
		p.cptr -= sizeof(void*);
		GMint allocId = *p.iptr;

		bool found = false;

		for (GMint i = 0; i < mynumallocs; i++)
		{
			if (allocations_id[i] == allocId)
			{
				allocations_id[i] = allocations_id[mynumallocs - 1];
				allocations_bytes[i] = allocations_bytes[mynumallocs - 1];
				mynumallocs--;
				found = true;
				break;
			}
		}


		gm_s_totalBytesAlignedAllocs -= size;

		GMint diff = gm_s_numAlignedAllocs - gm_s_numAlignedFree;
		gm_s_freeFunc(real);
	}
}
#else
void* AlignedMemoryAlloc::gmAlignedAllocInternal(size_t size, GMint alignment)
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
#endif