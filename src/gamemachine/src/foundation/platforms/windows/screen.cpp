#include "stdafx.h"
#include "check.h"
#include <gmtools.h>

GMint32 GMScreen::horizontalResolutionDpi()
{
	static GMint32 dpi = -1;
	if (dpi < 0)
	{
		HDC hdcScreen = GetDC(0);
		dpi = GetDeviceCaps(hdcScreen, LOGPIXELSX);
		ReleaseDC(0, hdcScreen);
	}
	return dpi;
}

GMint32 GMScreen::verticalResolutionDpi()
{
	static GMint32 dpi = -1;
	if (dpi < 0)
	{
		HDC hdcScreen = GetDC(0);
		dpi = GetDeviceCaps(hdcScreen, LOGPIXELSY);
		ReleaseDC(0, hdcScreen);
	}
	return dpi;
}