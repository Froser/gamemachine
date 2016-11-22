#ifndef __ASSERT_H__
#include "common.h"
BEGIN_NS
#define ASSERT(cond) ((!(cond)) ? Assert::assert(#cond, __FILE__, __LINE__) : Assert::noop())

class Assert
{
public:
	static void noop();
	static void assert(const char *assertion, const char *file, int line);
};
END_NS
#endif