#ifndef __DEFINES_H__
#define __DEFINES_H__

// 容器别名
#if GM_MSVC
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

template <typename T1, typename T2, typename Cmp = std::less<T1> >
using Map = std::map<T1, T2, Cmp>;

template <typename T1, typename T2, typename Cmp = std::less<T1> >
using Multimap = std::multimap<T1, T2, Cmp>;

template <typename T, typename Cmp = std::less<T> >
using Set = std::set<T, Cmp>;

template <typename T>
using Vector = std::vector<T>;

template <typename T>
using Queue = std::queue<T>;

template <typename T, size_t size>
using Array = std::array<T, size>;

template <typename T>
using Hash = std::hash<T>;

template <typename T1, typename T2, typename Hasher = Hash<T1>>
using HashMap = std::unordered_map<T1, T2, Hasher>;

template <typename T>
using Stack = std::stack<T>;

template <typename T>
using List = std::list<T>;

template <typename T>
using Deque = std::deque<T>;

template <typename... T>
using Tuple = std::tuple<T...>;

template <typename T>
void GMClearSTLContainer(T& c)
{
	T empty;
	using std::swap;
	swap(c, empty);
}

/* 工程编译选项 */
// 一定会使用使用OpenGL
#ifndef GM_USE_OPENGL
#	define GM_USE_OPENGL 1
#endif

// 是否使用DirectX11
#ifndef GM_USE_DX11
#	define GM_USE_DX11 0
#else
#	if _DEBUG
#		define D3D_DEBUG_INFO
#	endif
#endif

// 编译设置：
#ifndef GM_LIB
#	define GM_LIB 1
#endif

/* 工程编译选项到此结束 */

#if GM_DETECT_MEMORY_LEAK
#	if GM_WINDOWS
#		include <vld.h> // Windows环境下，确保安装了VLD，否则请将GM_DETECT_MEMORY_LEAK从CMake中关闭
#	endif
#endif

#if GM_MSVC
#	define GM_NOVTABLE __declspec(novtable)
#else
#	define GM_NOVTABLE
#endif

#define GM_ALIGNED_16(t) t __declspec(align(16))

#if GM_LIB
#	define GM_API
#	define GM_LUA_API extern
#else
#	define GM_API
#	define GM_LUA_API extern
#	error consider dll
#endif

#if GM_USE_OPENGL
#	ifndef GLEW_STATIC
#		define GLEW_STATIC
#	endif
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

// 表示作为数据输入
#define IN

// 用于枚举的宏
#define GM_FOREACH_ENUM(var, start, end) for (auto var = start; var < end; var = (decltype(var))(((gm::GMint)var)+1) )

#define GM_FOREACH_ENUM_CLASS(var, start, end) for (decltype(start) var = start; (gm::GMint) var < (gm::GMint) end; var = (decltype(var))(((gm::GMint)var)+1) )

// 非WINDOWS下的数据类型
#if !GM_WINDOWS && !GM_MSVC
typedef unsigned short WORD;
typedef unsigned long DWORD;
GM_STATIC_ASSERT_SIZE(WORD, 2);
GM_STATIC_ASSERT_SIZE(DWORD, 4);
#endif

BEGIN_NS
// 基本数据类型
typedef float GMfloat;
typedef int GMint;
typedef unsigned int GMuint;
typedef short GMshort;
typedef ptrdiff_t GMptrdiff;
typedef unsigned char GMbyte;
typedef long GMlong;
typedef unsigned long GMulong;
typedef short GMshort;
typedef unsigned short GMushort;
typedef wchar_t GMwchar;
typedef int64_t GMint64;
typedef GMint GMFontSizePt;
typedef size_t GMsize_t;

// 类型大小静态断言，如果在某些环境下失败，应该同步typedef使得其编译通过
#define GM_STATIC_ASSERT static_assert
#define GM_STATIC_ASSERT_SIZE(type, size) GM_STATIC_ASSERT(sizeof(type) == size, "Type '" #type "' size static assert failed. Excepted size is " #size);
GM_STATIC_ASSERT_SIZE(GMbyte, 1);
GM_STATIC_ASSERT_SIZE(GMint, 4);
GM_STATIC_ASSERT_SIZE(GMuint, 4);
GM_STATIC_ASSERT_SIZE(GMfloat, 4);
GM_STATIC_ASSERT_SIZE(GMint64, 8);

// 常用函数和工具、常量
struct GMRect
{
	GMint x, y, width, height;
};

struct GMRectF
{
	GMfloat x, y, width, height;
};

struct GMPoint
{
	GMint x;
	GMint y;
};

template <typename T>
inline bool GM_inRect(const T& rect, const decltype(T().x) x, const decltype(T().y) y)
{
	return (x > rect.x) && (x < rect.x + rect.width) &&
		(y > rect.y) && (y < rect.y + rect.height);
}

inline bool GM_inRect(const GMRect& rect, const GMPoint& pt)
{
	return (pt.x > rect.x) && (pt.x < rect.x + rect.width) &&
		(pt.y > rect.y) && (pt.y < rect.y + rect.height);
}

inline bool operator==(const GMRect& lhs, const GMRect& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.width == rhs.width && lhs.height == rhs.height;
}

inline bool operator!=(const GMRect& lhs, const GMRect& rhs)
{
	return !(lhs == rhs);
}

template <typename T>
inline void GM_delete(T*& o)
{
	if (o)
	{
		delete o;
#if _DEBUG
		o = nullptr;
#endif
	}
}

template <typename T>
inline void GM_delete(T*&& o)
{
	delete o;
}

template <typename T>
inline void GM_delete(Vector<T*>& o)
{
	for (auto& obj : o)
	{
		GM_delete(obj);
	}
#if _DEBUG
	o.swap(Vector<T*>());
#endif
}

template <typename T>
inline void GM_delete_array(T*& o)
{
	if (o)
	{
		delete[] o;
#if _DEBUG
		o = nullptr;
#endif
	}
}

#define LINE_MAX 10240 * 4

#define GM_SWAP(a, b) { auto t = a; a = b; b = t; }

#define GM_ZeroMemory(dest) memset((dest), 0, sizeof(dest));
#define GM_array_size(i) ( sizeof((i)) / sizeof((i)[0]) )

#if GM_WINDOWS
#	define GM_CRLF L"\r\n"
#else
#	define GM_CRLF L"\n"
#endif

END_NS

// 全局空间
// 平台差异
#if !GM_MSVC
#	ifdef SAFE_SSCANF
#		undef SAFE_SSCANF
#		define SAFE_SSCANF(in, format, out)	\
	{										\
		const char* _str = in;				\
		if (_str)							\
			sscanf(_str, format, out);		\
	}
#	endif

inline void fopen_s(FILE** f, const char* filename, const char* mode)
{
	*f = fopen(filename, mode);
}

inline void memcpy_s(void* dest, size_t, const void* src, size_t size)
{
	memcpy(dest, src, size);
}

#endif

#endif
