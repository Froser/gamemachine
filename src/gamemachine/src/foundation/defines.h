#ifndef __DEFINES_H__
#define __DEFINES_H__

/* 工程编译选项 */
#ifndef GM_DEBUG
#	if GM_WINDOWS
#		if DEBUG
#			define GM_DEBUG 1
#		elif _DEBUG
#			define GM_DEBUG 1
#		endif
#	elif GM_UNIX
#		ifdef GM_DEBUG_FLAG
#			define GM_DEBUG 1
#			undef NDEBUG
#		else
#			ifndef NDEBUG
#				define NDEBUG
#			endif
#		endif
#	endif
#endif

// 导入导出
#pragma warning( disable: 4251 )

#ifndef GM_DECL_EXPORT
#	ifdef GM_WINDOWS
#		define GM_DECL_EXPORT __declspec(dllexport)
#	elif GM_GCC
#		define GM_DECL_EXPORT __attribute__((visibility("default")))
#	endif
#	ifndef GM_DECL_EXPORT
#		define GM_DECL_EXPORT
#	endif
#endif
#ifndef GM_DECL_IMPORT
#	if GM_WINDOWS
#		define GM_DECL_IMPORT __declspec(dllimport)
#	else
#		define GM_DECL_IMPORT
#	endif
#endif

#ifdef GM_DLL
#	ifndef GM_EXPORT
#		define GM_EXPORT GM_DECL_EXPORT
#	endif
#else
#	if GM_USE_DLL
#		ifndef GM_EXPORT
#			define GM_EXPORT GM_DECL_IMPORT
#		endif
#	else
#		ifndef GM_EXPORT
#			define GM_EXPORT
#		endif
#	endif
#endif

// 容器别名
#if GM_WINDOWS
#	include <wtypes.h>
#else
#	include <string.h> //memset
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
#include <memory>
#include <atomic>
#include <limits>
#include "assert.h"

#if GM_UNIX
#include <pthread.h>
#endif

template <typename T1, typename T2>
using Pair = std::pair<T1, T2>;

template <typename T1, typename T2, typename Cmp = std::less<T1>, typename Alloc = std::allocator<std::pair<const T1, T2>> >
using Map = std::map<T1, T2, Cmp, Alloc>;

template <typename T1, typename T2, typename Cmp = std::less<T1> >
using Multimap = std::multimap<T1, T2, Cmp>;

template <typename T, typename Cmp = std::less<T> >
using Set = std::set<T, Cmp>;

template <typename T, typename Alloc = std::allocator<T>>
using Vector = std::vector<T, Alloc>;

template <typename T>
using Queue = std::queue<T>;

template <typename T, size_t size>
using Array = std::array<T, size>;

template <typename T>
using Hash = std::hash<T>;

template <typename T1, typename T2, typename Hasher = Hash<T1>>
using HashMap = std::unordered_map<T1, T2, Hasher>;

template <typename T, typename Alloc = std::allocator<T>>
using Deque = std::deque<T, Alloc>;

template <typename T, typename Container = Deque<T> >
using Stack = std::stack<T, Container>;

template <typename T>
using List = std::list<T>;

template <typename... T>
using Tuple = std::tuple<T...>;

template <typename T>
void GMClearSTLContainer(T& c)
{
	T empty;
	using std::swap;
	swap(c, empty);
}

// Atomic
template <typename T>
using GMAtomic = std::atomic<T>;

// 一定会使用使用OpenGL
#ifndef GM_USE_OPENGL
#	define GM_USE_OPENGL 1
#endif

// 是否使用DirectX11
#ifndef GM_USE_DX11
#	define GM_USE_DX11 0
#else
#	if GM_DEBUG
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

// 定义对齐结构体
#if GM_WINDOWS
#	define GM_ALIGNED_16(t) t __declspec(align(16))
#else
#	define GM_ALIGNED_16(t) t __attribute__((aligned(16)))
#endif

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

#ifdef min
#undef min
#endif

#ifdef max
#undef max
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

// 表示一个方法是个元方法
#define GM_META_METHOD static

// 用于枚举的宏
#define GM_FOREACH_ENUM(var, start, end) for (auto var = start; var < end; var = (decltype(var))(((gm::GMint32)var)+1) )

#define GM_FOREACH_ENUM_CLASS(var, start, end) for (decltype(start) var = start; (gm::GMint32) var < (gm::GMint32) end; var = (decltype(var))(((gm::GMint32)var)+1) )

// 字符串化
#define __GM_STRINGIFY_L( _x ) L ## # _x
#define GM_STRINGIFY_L( _x ) __GM_STRINGIFY_L( _x )

// 非WINDOWS下的数据类型
#if GM_WINDOWS
typedef WORD GMWord;
typedef DWORD GMDWord;
#else
typedef unsigned short GMWord;
typedef unsigned int GMDWord;
#endif

#if GM_WINDOWS
#	define GM_SYSTEM_CALLBACK CALLBACK
#else
#	define GM_SYSTEM_CALLBACK
#endif

#define GM_NOEXCEPT noexcept

BEGIN_NS
// 基本数据类型
typedef float GMfloat;
typedef GMfloat GMDuration;
typedef int GMint32;
typedef unsigned int GMuint32;
typedef short GMshort;
typedef ptrdiff_t GMptrdiff;
typedef unsigned char GMbyte;
typedef long GMlong;
typedef unsigned long GMulong;
typedef short GMshort;
typedef unsigned short GMushort;
typedef wchar_t GMwchar;
typedef int64_t GMint64;
typedef GMint32 GMFontSizePt;
typedef size_t GMsize_t;

#if GM_WINDOWS
typedef WPARAM GMWParam;
typedef LPARAM GMLParam;
typedef LRESULT GMLResult;
#else
typedef GMint32 GMWParam;
typedef GMlong GMLParam;
typedef GMlong GMLResult;
#endif

// 线程
#if GM_WINDOWS
typedef void* GMThreadHandle;
typedef GMlong GMThreadId;
#elif GM_UNIX
// USE pthread
typedef pthread_t GMThreadHandle;
typedef pthread_t GMThreadId;
typedef pthread_attr_t GMThreadAttr;
#endif

// 类型大小静态断言，如果在某些环境下失败，应该同步typedef使得其编译通过
#define GM_STATIC_ASSERT static_assert
#define GM_STATIC_ASSERT_SIZE(type, size) GM_STATIC_ASSERT(sizeof(type) == size, "Type '" #type "' size static assert failed. Excepted size is " #size);
GM_STATIC_ASSERT_SIZE(GMbyte, 1);
GM_STATIC_ASSERT_SIZE(GMint32, 4);
GM_STATIC_ASSERT_SIZE(GMuint32, 4);
GM_STATIC_ASSERT_SIZE(GMfloat, 4);
GM_STATIC_ASSERT_SIZE(GMint64, 8);
GM_STATIC_ASSERT_SIZE(GMWord, 2);
GM_STATIC_ASSERT_SIZE(GMDWord, 4);

// 常用函数和工具、常量
template <typename T, typename DeleteFunc = std::default_delete<T>>
using GMOwnedPtr = std::unique_ptr<T, DeleteFunc>;

template <typename T>
using GMSharedPtr = std::shared_ptr<T>;

#define GM_OWNED
#define gm_makeOwnedPtr std::make_unique
#define gm_makeSharedPtr std::make_shared

typedef GMuint32 GMFontHandle;
constexpr GMuint32 GMInvalidFontHandle = -1;

struct GMRect
{
	GMint32 x, y, width, height;
};

struct GMRectF
{
	GMfloat x, y, width, height;
};

struct GMPoint
{
	GMint32 x;
	GMint32 y;
};

inline GMint32 gm_sizet_to_int(GMsize_t sz)
{
	GM_ASSERT(sz < static_cast<GMsize_t>(std::numeric_limits<GMint32>::max()));
	return static_cast<GMint32>(sz);
}

inline GMuint32 gm_sizet_to_uint(GMsize_t sz)
{
	GM_ASSERT(sz < static_cast<GMsize_t>(std::numeric_limits<GMuint32>::max()));
	return static_cast<GMuint32>(sz);
}

template <typename T>
inline T gm_sizet_to(GMsize_t sz)
{
	GM_ASSERT(sz < static_cast<GMsize_t>(std::numeric_limits<T>::max()));
	return static_cast<T>(sz);
}

template <typename T>
inline bool GM_inRect(const T& rect, const decltype(T().x) x, const decltype(T().y) y)
{
	return (x >= rect.x) && (x <= rect.x + rect.width) &&
		(y >= rect.y) && (y <= rect.y + rect.height);
}

inline bool GM_inRect(const GMRect& rect, const GMPoint& pt)
{
	return (pt.x >= rect.x) && (pt.x <= rect.x + rect.width) &&
		(pt.y >= rect.y) && (pt.y <= rect.y + rect.height);
}

inline GMRect GM_intersectRect(const GMRect& rect1, const GMRect& rect2)
{
	GMRect rc;

	rc.x = rect1.x > rect2.x ? rect1.x : rect2.x;
	rc.y = rect1.y > rect2.y ? rect1.y : rect2.y;
	GMint32 right = (rect1.x + rect1.width) < (rect2.x + rect2.width) ? (rect1.x + rect1.width) : (rect2.x + rect2.width);
	GMint32 bottom = (rect1.y + rect1.height) < (rect2.y + rect2.height) ? (rect1.y + rect1.height) : (rect2.y + rect2.height);
	rc.width = right - rc.x;
	rc.height = bottom - rc.y;
	if (rc.width < 0)
		rc.width = 0;
	if (rc.height < 0)
		rc.height = 0;
	return rc;
}

inline GMRect GM_unionRect(const GMRect& rect1, const GMRect& rect2)
{
	static auto Min = [](GMint32 x, GMint32 y) { return x < y ? x : y; };
	static auto Max = [](GMint32 x, GMint32 y) { return x > y ? x : y; };

	GMRect rc;
	rc.x = Min(rect1.x, rect2.x);
	rc.y = Min(rect1.y, rect2.y);
	rc.width = Max(rect1.x + rect1.width, rect2.x + rect2.width) - rc.x;
	rc.height = Max(rect1.y + rect1.height, rect2.y + rect2.height) - rc.y;
	return rc;
}

inline bool operator==(const GMRect& lhs, const GMRect& rhs)
{
	return lhs.x == rhs.x && lhs.y == rhs.y && lhs.width == rhs.width && lhs.height == rhs.height;
}

inline bool operator!=(const GMRect& lhs, const GMRect& rhs)
{
	return !(lhs == rhs);
}

//! IDestroyObject接口无其它含义，表示此类有一个虚析构函数
/*!
继承此接口，将自动获得一个虚析构函数。
*/
struct GM_EXPORT IDestroyObject
{
	virtual ~IDestroyObject() = default;
	virtual void destroy() { delete this; }
};

inline void GM_delete(IDestroyObject*& o)
{
	if (o)
	{
		o->destroy();
		o = nullptr;
	}
}

template <typename T>
inline void GM_delete(T*& o)
{
	if (o)
	{
		delete o;
		o = nullptr;
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
	GMClearSTLContainer(o);
}

template <typename T>
inline void GM_delete_array(T*& o)
{
	if (o)
	{
		delete[] o;
		o = nullptr;
	}
}

#define LINE_MAX 10240 * 4

#define GM_SWAP(a, b) { auto t = a; a = b; b = t; }

template <typename T, GMsize_t S>
inline void GM_ZeroMemory(T(&dest)[S])
{
	memset(dest, 0, S);
}

inline void GM_ZeroMemory(void* dest, GMsize_t size)
{
	memset(dest, 0, size);
}

#define GM_array_size(i) ( sizeof((i)) / sizeof((i)[0]) )

#if GM_WINDOWS
#	define GM_NEXTLINE L"\r\n"
#else
#	define GM_NEXTLINE L"\n"
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

#	define GM_MAX_PATH 260
#	define GM_CDECL __attribute__((cdecl))
#	define GM_STDCALL __attribute__((stdcall))
#else
#	define GM_CDECL __cdecl
#	define GM_STDCALL __stdcall
#	define GM_MAX_PATH MAX_PATH
#endif

#endif
