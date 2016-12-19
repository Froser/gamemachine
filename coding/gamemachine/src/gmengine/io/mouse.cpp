#include "stdafx.h"
#include "mouse.h"
#ifdef _WINDOWS
#	include <windows.h>
#endif

void Mouse::getCursorPosition(int* x, int* y)
{
#ifdef _WINDOWS
	POINT pos;
	::GetCursorPos(&pos);
	*x = pos.x;
	*y = pos.y;
#else
	ASSERT(false);
#endif
}

void Mouse::setCursorPosition(int x, int y)
{
#ifdef _WINDOWS
	::SetCursorPos(x, y);
#else
	ASSERT(false);
#endif
}

void Mouse::showCursor(bool show)
{
#ifdef _WINDOWS
	::ShowCursor(show ? TRUE : FALSE);
#else
	ASSERT(false);
#endif
}