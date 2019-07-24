#include "stdafx.h"
#include "check.h"
#include "defines.h"
#include "foundation/assert.h"
#include <time.h>

BEGIN_NS
extern "C"
{
	GMint64 highResolutionTimerFrequency()
	{
		// In unix, we use it as a unit-conversion.
		return 1000000000;
	}

	GMint64 highResolutionTimer()
	{
		struct timespec now;
		clock_gettime(CLOCK_MONOTONIC, &now);
		return now.tv_sec * highResolutionTimerFrequency() + now.tv_nsec;
	}
}
END_NS