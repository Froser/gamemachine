#ifndef __ASSERT_H__
#define __ASSERT_H__
#include <type_traits>

#if GM_WINDOWS
#	if GM_DEBUG
#		define GM_ASSERT(cond) ((!(cond)) ? gm::Assert::gm_assert(#cond, __FILE__, __LINE__) : gm::Assert::noop())
#	else
#		define GM_ASSERT(cond)
#	endif

namespace gm
{
	class Assert
	{
	public:
		static void noop();
		static void gm_assert(const char *assertion, const char *file, int line);
	};
}

#elif GM_UNIX
#	include <assert.h>
#	define GM_ASSERT(cond) assert(cond)
#endif

#endif