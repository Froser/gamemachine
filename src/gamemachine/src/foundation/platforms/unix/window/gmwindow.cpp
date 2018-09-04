#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
#include <X11/Xlib.h>
#include <GL/glx.h>
#include "gminput.h"

GM_PRIVATE_OBJECT(GMXRenderContext)
{
	Screen* screen;
	GMint screenWidth, screenHeight, screenWidthMM, screenHeightMM;
};

GMWindow::GMWindow()
{
}

IInput* GMWindow::getInputMananger()
{
	D(d);
	if (!d->input)
		d->input = gm_makeOwnedPtr<GMInput>(this);
	return d->input.get();
}