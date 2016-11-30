#define BEGIN_NS namespace gm {
#define END_NS }
#define DEFINE_PRIVATE(className) \
	private: \
	className##Private m_data; \
	className##Private& dataRef() { return m_data; }

#ifndef _WINDOWS
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef long LONG;
typedef unsigned char BYTE;
#endif

#define GLEW_STATIC
#include "GL/glew.h"

// Options
typedef unsigned char FByte;
typedef GLfloat Ffloat;
typedef GLint Fint;
typedef GLuint Fuint;
#define LINE_MAX 256