#ifndef __DEFINES_H__
#define __DEFINES_H__

// 容器别名
#if _MSC_VER
#	include <wtypes.h>
#else
#	include <strings.h>
#endif
#include <utility>
#include <map>
#include <set>
#include <vector>
#include <queue>
#include <array>
#include <unordered_map>
#include <stack>
#include <list>

template <typename T1, typename T2>
using Pair = std::pair<T1, T2>;
#define makePair std::make_pair

template <typename T1, typename T2>
using Map = std::map<T1, T2>;

template <typename T1, typename T2>
using Multimap = std::multimap<T1, T2>;

template <typename T>
using Set = std::set<T>;

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using Queue = std::queue<T>;

template <typename T, size_t size>
using Array = std::array<T, size>;

template <typename T>
using Hash = std::hash<T>;

template <typename T1, typename T2>
using UnorderedMap = std::unordered_map<T1, T2>;

template <typename T>
using Stack = std::stack<T>;

template <typename T>
using List = std::list<T>;

template <typename T>
using Deque = std::deque<T>;

/* 工程编译选项 */
// 使用UNICODE
#ifndef UNICODE
#	define UNICODE 1
#endif

// 使用OpenGL
#define USE_OPENGL 1

// 编译设置：
#ifndef GM_LIB
#	define GM_LIB 1
#endif

// 游戏逻辑的线程模式
#ifndef GM_MULTI_THREAD
#	define GM_MULTI_THREAD 1
#endif

// Debug模式下监控内存泄漏
#ifndef GM_DETECT_MEMORY_LEAK
#	define GM_DETECT_MEMORY_LEAK 0
#endif

// SSE指令优化
#ifndef GM_SIMD
#	define GM_SIMD 1
#endif

/* 工程编译选项到此结束 */

#if GM_DETECT_MEMORY_LEAK
#	if _WINDOWS
#		include <vld.h> // Windows环境下，确保安装了VLD，否则请将GM_DETECT_MEMORY_LEAK设置为0
#	endif
#endif

#if _MSC_VER
#	define GM_NOVTABLE __declspec(novtable)
#else
#	define GM_NOVTABLE
#endif

#if _MSC_VER && GM_SIMD
#	define USE_SIMD 1
#else
#	define USE_SIMD 0
#endif

#if USE_SIMD
#	define GM_ALIGNED_16(t) t __declspec(align(16))
#else
#	define GM_ALIGNED_16(t) t
#endif

#if GM_LIB
#	define GM_API
#	define GM_LUA_API extern
#else
#	define GM_API
#	define GM_LUA_API extern
#	error consider dll
#endif

#if USE_OPENGL
#	define GLEW_STATIC
#	include "GL/glew.h"
#endif

// 整个GameMachine用到的宏定义
#define _L(x) L ## x

#define BEGIN_NS namespace gm {
#define END_NS }

// 表示此变量会被自动释放
#define AUTORELEASE

// 表示此变量会在函数内被分配到堆上，需要自己释放
#define OUT

// 表示是个引用，函数会改变此变量的值
#define REF

// 表示作为数据输入
#define IN

// 用于枚举的宏
#define GM_FOREACH_ENUM(var, start, end) for (auto var = start; var < end; var = (decltype(var))(((gm::GMint)var)+1) )

#define GM_FOREACH_ENUM_CLASS(var, start, end) for (decltype(start) var = start; (GMint) var < (GMint) end; var = (decltype(var))(((GMint)var)+1) )

#define GM_ENUM_END(e) (decltype(e)) ((GMint)(e) + 1)

BEGIN_NS

// 基本数据类型
typedef unsigned char GMbyte;
typedef long GMlong;
typedef unsigned long GMulong;
typedef short GMshort;
typedef unsigned short GMushort;
typedef wchar_t GMWchar;
typedef int64_t GMLargeInteger;

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

#if !_MSC_VER
typedef unsigned short WORD;
typedef unsigned long DWORD;
#endif

// 类型大小静态断言，如果在某些环境下失败，应该同步typedef使得其编译通过
#define STATIC_ASSERT_SIZE(type, size) static_assert(sizeof(type) == size, "Type '" #type "' size static assert failed. Excepted size is " #size);
STATIC_ASSERT_SIZE(GMbyte, 1);
STATIC_ASSERT_SIZE(GMint, 4);
STATIC_ASSERT_SIZE(GMuint, 4);
STATIC_ASSERT_SIZE(GMfloat, 4);
STATIC_ASSERT_SIZE(GMLargeInteger, 8);

// 常用函数和工具、常量
struct GMRect
{
	GMint x, y, width, height;
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

#define LINE_MAX 10240
#define PI 3.141592653f
#define HALF_PI 1.5707963265f
#define RAD(deg) deg * (PI / 180)
#define DEG(rad) rad * (180 / PI)
#define SQR(a) ((a) * (a))
#define strEqual(str1, str2) !strcmp(str1, str2)
#define wstrEqual(str1, str2) !wcscmp(str1, str2)
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

#define GM_ZeroMemory(dest) memset((dest), 0, sizeof(dest));

END_NS

// 全局空间
// 平台差异
#if !_MSC_VER
#	ifdef SAFE_SSCANF
#		undef SAFE_SSCANF
#		define SAFE_SSCANF(in, format, out)	\
	{										\
		const char* _str = in;				\
		if (_str)							\
			sscanf(_str, format, out);		\
	}
#	endif

#define _stricmp strcasecmp

inline gm::GMWchar* wcscpy_s(gm::GMWchar* dest,  const gm::GMWchar* source)
{
	return wcscpy(dest, source);
}

inline gm::GMWchar* wcscpy_s(gm::GMWchar* dest, size_t, const gm::GMWchar* source)
{
	return wcscpy(dest, source);
}

inline void strcpy_s(char* dest, size_t len, const char* source)
{
	strcpy(dest, source);
}

inline void strcpy_s(char* dest, const char* source)
{
	strcpy(dest, source);
}

inline void fopen_s(FILE** f, const char* filename, const char* mode)
{
	*f = fopen(filename, mode);
}

inline void memcpy_s(void* dest, size_t, const void* src, size_t size)
{
	memcpy(dest, src, size);
}

inline char* strcat_s(char* dest, size_t, const char* src)
{
	return strcat(dest, src);
}

inline char* strncpy_s(char* dest, size_t, const char* src, size_t count)
{
	return strncpy(dest, src, count);
}

#endif

#endif
