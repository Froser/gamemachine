#define BEGIN_NS namespace fglextlib {
#define END_NS }
#define DEFINE_PRIVATE(className) \
	private: \
	className##Private m_data; \
	className##Private& dataRef() { return m_data; }

#ifdef FGLEXTLIB
#	define FGLEXTLIBAPI __declspec(dllexport)
#else
#	define FGLEXTLIBAPI __declspec(dllimport)
#endif

// Options
#include "gl/GL.h"
#define Ffloat GLfloat
#define Fint GLint
#define LINE_MAX 256
