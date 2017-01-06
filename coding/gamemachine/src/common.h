#define BEGIN_NS namespace gm {
#define END_NS }
#define DEFINE_PRIVATE(className) \
	private: \
	className##Private m_data; \
	protected: \
	className##Private& dataRef() { return m_data; }

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
#endif

#define FREEGLUT_STATIC
#define GLEW_STATIC
#include "GL/glew.h"

typedef unsigned char GMbyte;
typedef GLfloat GMfloat;
typedef GLint GMint;
typedef GLuint GMuint;
typedef GLsizei GMsizei;
typedef GLsizeiptr GMsizeiptr;
typedef GMuint GMvertexoffset;
#define LINE_MAX 256

//Tools:
#define PI 3.141592653f
#define HALF_PI 1.5707963265f
#define RAD(deg) deg * PI / 180
#define strEqual(str1, str2) !strcmp(str1, str2)