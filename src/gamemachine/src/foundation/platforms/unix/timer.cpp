#include "stdafx.h"
#include "check.h"
#include "defines.h"
#include "foundation/assert.h"

extern "C"
{
	GMint64 highResolutionTimerFrequency()
	{
		return 0;
	}

	GMint64 highResolutionTimer()
	{
		return 0;
	}
}