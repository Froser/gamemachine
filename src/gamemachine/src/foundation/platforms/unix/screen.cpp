#include "stdafx.h"
#include "check.h"
#include <gmtools.h>

GMfloat GMScreen::dpi()
{
	static GMint dpi = -1;
	if (dpi < 0)
	{
		dpi = 1; //TODO
	}
	return dpi;
}