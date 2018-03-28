#include "stdafx.h"
#include "gmmcursor.h"

bool GMUICursor::createCursor(void* hinstance, const gm::GMImage& cursorImg)
{
	D(d);
	gm::GMint cursorWidth = GetSystemMetrics(SM_CXCURSOR);
	gm::GMint cursorHeight = GetSystemMetrics(SM_CYCURSOR);
}
