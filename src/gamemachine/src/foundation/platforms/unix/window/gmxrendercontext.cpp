#include "stdafx.h"
#include "gmxrendercontext.h"
#include <GL/glx.h>

Display* GMXRenderContext::s_display = nullptr;

GMXRenderContext::GMXRenderContext(const char* displayName)
{
	D(d);
	if (!s_display)
	{
		s_display = XOpenDisplay(displayName);
		XSynchronize(s_display, True);
		if (!glXQueryExtension(s_display, NULL, NULL))
			gm_error(gm_dbg_wrap("OpenGL GLX extension not supported by s_display '{0}"), XDisplayName(displayName));
	}
	d->screen = DefaultScreen(s_display);
	d->rootWindow = RootWindow(s_display, d->screen);
	d->screenWidth = DisplayWidth(s_display);
	d->screenHeight = DisplayHeight(s_display);
	d->screenWidthMM = DisplayWidthMM(s_display);
	d->screenHeightMM = DisplayHeightMM(s_display);
}
