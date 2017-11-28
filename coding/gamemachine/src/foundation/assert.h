#ifndef __ASSERT_H__
#define __ASSERT_H__
#include "defines.h"
#include <type_traits>

#if _DEBUG
#define GM_ASSERT(cond) ((!(cond)) ? gm::Assert::gm_assert(#cond, __FILE__, __LINE__) : gm::Assert::noop())
#else
#define GM_ASSERT(cond)
#endif

template <typename T, typename U>
T gm_static_cast(U another)
{
#if _DEBUG
	GM_STATIC_ASSERT(std::is_pointer<T>::value, "T must be a pointer");
	T ptr = dynamic_cast<T>(another);
	GM_ASSERT(ptr);
	return ptr;
#else
	return static_cast<T>(another);
#endif
}

BEGIN_NS
class Assert
{
public:
	static void noop();
	static void gm_assert(const char *assertion, const char *file, int line);
};

END_NS
#endif