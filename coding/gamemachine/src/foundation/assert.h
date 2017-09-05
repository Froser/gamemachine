#ifndef __ASSERT_H__
#define __ASSERT_H__
#include "defines.h"

#if _DEBUG
#define GM_ASSERT(cond) ((!(cond)) ? gm::Assert::gm_assert(#cond, __FILE__, __LINE__) : gm::Assert::noop())
#else
#define GM_ASSERT(cond)
#endif

BEGIN_NS
class Assert
{
public:
	static void noop();
	static void gm_assert(const char *assertion, const char *file, int line);
};

END_NS
#endif