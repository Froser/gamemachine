#include "stdafx.h"
#include "check.h"
#include <gmtools.h>

GMfloat GMScreen::dpi()
{
	static GMint dpi = -1;
	if (dpi < 0)
	{
		HDC hdcScreen = GetDC(0);
		dpi = GetDeviceCaps(hdcScreen, LOGPIXELSX);
		ReleaseDC(0, hdcScreen);
	}
	return dpi;
}