#ifndef __COMMON_H__
#define __COMMON_H__

#pragma warning(disable:4244)

#define BEGIN_NS namespace gm {
#define END_NS }
#define DEFINE_PRIVATE(className) \
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

#define FREEGLUT_STATIC
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
#endif