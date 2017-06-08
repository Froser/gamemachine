#ifndef __DEFINES_H__
#define __DEFINES_H__

// 使用UNICODE
#ifndef UNICODE
#	define UNICODE 1
#endif
#define _L(x) L ## x

// 使用OpenGL
#define USE_OPENGL 1

// 如果是OpenGL，加载glew
#if USE_OPENGL
#	define GLEW_STATIC
#	include "GL/glew.h"
#endif

// 整个GameMachine用到的宏定义

#define BEGIN_NS namespace gm {
#define END_NS }

// 表示此变量会被自动释放
#define AUTORELEASE

// 表示此变量会在函数内被分配到堆上，需要自己释放
#define OUT

// 表示是个引用，函数会改变此变量的值
#define REF

// 用于枚举的宏
#define BEGIN_ENUM(var, start, end) for (auto var = start; var < end; var = (decltype(var))(((GMuint)var)+1) )
#define END_ENUM

// 是否支持C++11
#if _MSC_VER <= 1600
#	define CPP11 0
#else
#	define CPP11 1
#endif

BEGIN_NS

// 基本数据类型
typedef unsigned char GMbyte;
typedef long GMlong;
typedef short GMshort;
typedef wchar_t GMWchar;
typedef __int64 GMLargeInteger;

#if USE_OPENGL
typedef GLfloat GMfloat;
typedef GLint GMint;
typedef GLuint GMuint;
typedef GLsizei GMsizei;
typedef GLsizeiptr GMsizeiptr;
#else
typedef float GMfloat;
typedef int GMint
typedef short GMshort;
#endif

// 常用函数和工具、常量
struct GMRect
{
	GMfloat x, y, width, height;
};

template <typename T>
void GM_new(OUT T** out)
{
	*out = new T;
	memset(*out, 0, sizeof(T));
}

template <typename T>
void GM_new_arr(OUT T** out, GMint cnt)
{
	*out = new T[cnt];
	memset(*out, 0, cnt * sizeof(T));
}

#define LINE_MAX 2048
#define PI 3.141592653f
#define HALF_PI 1.5707963265f
#define RAD(deg) deg * (PI / 180)
#define DEG(rad) rad * (180 / PI)
#define SQR(a) ((a) * (a))
#define strEqual(str1, str2) !strcmp(str1, str2)
#define SAFE_SSCANF(in, format, out)	\
{										\
	const char* _str = in;				\
	if (_str)							\
		sscanf_s(_str, format, out);	\
}

#define SAFE_SWSCANF(in, format, out)	\
{										\
	const GMWchar* _str = in;			\
	if (_str)							\
		swscanf_s(_str, format, out);	\
}
#define SWAP(a, b) { auto t = a; a = b; b = t; }

// 编译器相关
#if CPP11
#	define DEFAULT_CONSTRUCTOR(className) className() = default;
#else
#	define DEFAULT_CONSTRUCTOR(className) className(){}
#endif

// SSE指令优化
#ifdef _MSC_VER
#	define USE_SIMD 1
#else
#	define USE_SIMD 0
#endif

#if USE_SIMD
#	define GM_ALIGNED_16(t) t __declspec(align(16))
#	define _mm_madd_ps(a, b, c) _mm_add_ps(_mm_mul_ps((a), (b)), (c))
#	define simd_shuffle_param(x, y, z, w)  ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))
#	define simd_zeroMask (_mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f))
#	define simd_FFF0Mask (_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#else
#	define GM_ALIGNED_16(t) t
#endif

// 平台差异
#ifdef __GNUC__
#define strcat_s strcat

inline static void strcpy_s(GMchar* dest, size_t len, const GMchar* source)
{
	strcpy(dest, source);
}

inline static void strcpy_s(GMchar* dest, const GMchar* source)
{
	strcpy(dest, source);
}

inline static void fopen_s(FILE** f, const GMchar* filename, const GMchar* mode)
{
	*f = fopen(filename, mode);
}

#ifdef SAFE_SSCANF
#	undef SAFE_SSCANF
#	define SAFE_SSCANF(in, format, out)	\
{										\
	const GMchar* _str = in;				\
	if (_str)							\
		sscanf(_str, format, out);		\
}
#endif

#define MAX_PATH 256 // 最长路径
#define NO_LAMBDA //禁止使用Lambda表达式

#endif // __GNUC__

END_NS

#endif