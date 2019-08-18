#include "stdafx.h"
#include "check.h"
#include <gmtools.h>

BEGIN_NS

GMint32 GMScreen::horizontalResolutionDpi()
{
	static GMint32 dpi = -1;
	if (dpi < 0)
	{
		dpi = 96; //TODO
	}
	return dpi;
}

GMint32 GMScreen::verticalResolutionDpi()
{
	static GMint32 dpi = -1;
	if (dpi < 0)
	{
		dpi = 96; //TODO
	}
	return dpi;
}

END_NS