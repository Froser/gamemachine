#ifndef __DEFINES_H__
#define __DEFINES_H__

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

// 表示此变量会被自动释放
#define AUTORELEASE

// 表示此变量会在函数内被分配到堆上，需要自己释放
#define OUT

// 表示是个引用，函数会改变此变量的值
#define REF

// 用于枚举的宏
#define BEGIN_ENUM(var, start, end) for (auto var = start; var < end; var = (decltype(var))(((GMuint)var)+1) )
#define END_ENUM

BEGIN_NS

// 基本数据类型
typedef unsigned char GMbyte;
typedef GMbyte* PBYTE;
typedef long GMlong;
typedef short GMshort;
typedef wchar_t GMWChar;

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
#define SWAP(a, b) { auto t = a; a = b; b = t; }

//SSE指令优化
#ifdef _MSC_VER
#	define USE_SIMD 1
#	define GM_ALIGNED_16 __declspec(align(16))
#	define _mm_madd_ps(a, b, c) _mm_add_ps(_mm_mul_ps((a), (b)), (c))
#	define simd_shuffle_param(x, y, z, w)  ((x) | ((y) << 2) | ((z) << 4) | ((w) << 6))
#	define simd_zeroMask (_mm_set_ps(-0.0f, -0.0f, -0.0f, -0.0f))
#	define simd_FFF0Mask (_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF))
#else
#	define USE_SIMD 0
#	define GM_ALIGNED_16
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
#	undef SAFE_SSCANF
#	define SAFE_SSCANF(in, format, out)	\
{										\
	const char* _str = in;				\
	if (_str)							\
		sscanf(_str, format, out);		\
}
#endif

#define MAX_PATH 256 // 最长路径
#define NO_LAMBDA //禁止使用Lambda表达式

#endif // __APPLE__


END_NS

#endif