#include "stdafx.h"
#include "check.h"
#include <gmtools.h>

GMint GMScreen::horizontalResolutionDpi()
{
	static GMint dpi = -1;
	if (dpi < 0)
	{
		dpi = 96; //TODO
	}
	return dpi;
}

GMint GMScreen::verticalResolutionDpi()
{
	static GMint dpi = -1;
	if (dpi < 0)
	{
		dpi = 96; //TODO
	}
	return dpi;
}