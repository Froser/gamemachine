#ifndef __ASSERT_H__
#define __ASSERT_H__
#include "common.h"
BEGIN_NS
#define ASSERT(cond) ((!(cond)) ? Assert::assert(#cond, __FILE__, __LINE__) : Assert::noop())

class Assert
{
public:
	static void noop();
	static void assert(const char *assertion, const char *file, int line);
};

#ifdef USE_OPENGL
#define ASSERT_GL() ASSERT(glGetError() == GL_NO_ERROR)
#define CHECK_GL_LOC(i) ASSERT(i != -1) 
#else
#define ASSERT_GL()
#define CHECK_GL_LOC(i)
#endif
END_NS
#endif