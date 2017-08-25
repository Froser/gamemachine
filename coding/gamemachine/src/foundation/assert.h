#ifndef __ASSERT_H__
#define __ASSERT_H__
#include "common.h"
#include <string>

BEGIN_NS
#if _DEBUG
#define ASSERT(cond) ((!(cond)) ? Assert::gm_assert(#cond, __FILE__, __LINE__) : Assert::noop())
#else
#define ASSERT(cond)
#endif

#ifdef assert
#undef assert
#endif
class Assert
{
public:
	static void noop();
	static void gm_assert(const char *assertion, const char *file, int line);
};

END_NS
#endif