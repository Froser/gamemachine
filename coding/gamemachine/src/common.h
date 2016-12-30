#define BEGIN_NS namespace gm {
#define END_NS }
#define DEFINE_PRIVATE(className) \
	private: \
	className##Private m_data; \
	protected: \
	className##Private& dataRef() { return m_data; }

#define AUTORELEASE
#define OUT
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

// Options
typedef unsigned char GMbyte;
typedef GLfloat GMfloat;
typedef GLint GMint;
typedef GLuint GMuint;
typedef GLsizei GMsizei;
typedef GLsizeiptr GMsizeiptr;
typedef GMuint GMvertexoffset;
#define LINE_MAX 256

// Callbacks settings
#ifdef USE_OPENGL
#define GRAPHIC_ENGINE new GMGLGraphicEngine()
#endif