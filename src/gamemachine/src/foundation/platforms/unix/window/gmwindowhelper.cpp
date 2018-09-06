#include "stdafx.h"
#include "gmwindowhelper.h"

GMRect GMWindowHelper::getWindowRect(Display* display, Window window, Window parentWindow)
{
	GMRect rc;
	Window w;
	XTranslateCoordinates(display, window, parentWindow, 0, 0, &rc.x, &rc.y, &w);

	XWindowAttributes winAttributes;
	XGetWindowAttributes(
		display,
		window,
		&winAttributes
	);

	// width and height is not including border
	rc.width = winAttributes.width;
	rc.height = winAttributes.height;
	return rc;
}