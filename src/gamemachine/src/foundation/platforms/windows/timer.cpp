#include "stdafx.h"
#include "check.h"
#include "defines.h"
#include "foundation/assert.h"

BEGIN_NS
extern "C"
{
	GMint64 highResolutionTimerFrequency()
	{
		LARGE_INTEGER i;
		BOOL b = QueryPerformanceFrequency(&i);
		GM_ASSERT(b);
		return i.QuadPart;
	}

	GMint64 highResolutionTimer()
	{
		LARGE_INTEGER i;
		BOOL b = QueryPerformanceCounter(&i);
		GM_ASSERT(b);
		return i.QuadPart;
	}
}
END_NS