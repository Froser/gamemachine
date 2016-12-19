#ifndef __MOUSE_H__
#define __MOUSE_H__
#include "common.h"
BEGIN_NS

struct Mouse
{
	static void getCursorPosition(int* x, int* y);
	static void setCursorPosition(int x, int y);
	static void showCursor(bool show);
};

END_NS
#endif