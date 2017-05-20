#ifndef __COMMON_H__
#define __COMMON_H__
#include <stdio.h>
#include <string.h>

#pragma warning(disable:4244)

#define BEGIN_NS namespace gm {
#define END_NS }
#define DEFINE_PRIVATE(className) \
	public: \
	typedef className##Private Data; \
	private: \
	className##Private m_data; \
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

#define AUTORELEASE
#define OUT
#define REF
#define USE_OPENGL
#define OFFSET(type, count) (void*)(sizeof(type) * count)

#ifndef _WINDOWS
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned char BYTE;
typedef short SHORT;
#endif

#define GLEW_STATIC
#include "GL/glew.h"

BEGIN_NS
typedef unsigned char GMbyte;
typedef GMbyte* PBYTE;
typedef GLfloat GMfloat;
typedef GLint GMint;
typedef GLuint GMuint;
typedef GLsizei GMsizei;
typedef GLsizeiptr GMsizeiptr;
typedef long GMlong;
typedef short GMshort;
typedef wchar_t GMWChar;
struct GMRect
{
	GMfloat x, y, width, height;
};

template <typename T>
T* GM_new()
{
	T* t = new T;
	memset(t, 0, sizeof(T));
	return t;
}

template <typename T>
T* GM_new_arr(int cnt)
{
	T* t = new T[cnt];
	memset(t, 0, cnt * sizeof(T));
	return t;
}

#define BEGIN_ENUM(var, start, end) for (auto var = start; var < end; var = (decltype(var))(((GMuint)var)+1) )
#define END_ENUM

END_NS

#define LINE_MAX 2048

//Tools:
#define PI 3.141592653f
#define HALF_PI 1.5707963265f
#define RAD(deg) deg * PI / 180
#define DEG(rad) rad * 180 / PI
#define SQR(a) ((a) * (a))
#define strEqual(str1, str2) !strcmp(str1, str2)
#define SAFE_SSCANF(in, format, out)	\
{										\
	const char* _str = in;				\
	if (_str)							\
		sscanf_s(_str, format, out);	\
}
#define SWAP(a, b) { auto t = a; a = b; b = t; }

#ifdef _MSC_VER
#define USE_SIMD 1
#define GM_SIMD_float __declspec(align(16)) GMfloat
#define _mm_madd_ps(a, b, c) _mm_add_ps(_mm_mul_ps((a), (b)), (c))
#define simd_shuffle_param(x, y, z, w)  ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))
#else
#define USE_SIMD 0
#endif

// 平台差异
#ifdef __APPLE__
#define strcat_s strcat

inline static void strcpy_s(char* dest, size_t len, const char* source)
{
	strcpy(dest, source);
}

inline static void strcpy_s(char* dest, const char* source)
{
	strcpy(dest, source);
}

inline static void fopen_s(FILE** f, const char* filename, const char* mode)
{
	*f = fopen(filename, mode);
}

#ifdef SAFE_SSCANF
#undef SAFE_SSCANF
#define SAFE_SSCANF(in, format, out)	\
{										\
	const char* _str = in;				\
	if (_str)							\
		sscanf(_str, format, out);		\
}
#endif

#define MAX_PATH 256

#define NO_LAMBDA

#endif // __APPLE__

#endif
