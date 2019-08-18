#ifndef __GMWINDOW_HELPER_H__
#define __GMWINDOW_HELPER_H__
#include <gmcommon.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>

BEGIN_NS

struct GMWindowHelper
{
	static GMRect getWindowRect(Display* display, Window window, Window parentWindow);
};

END_NS
#endif