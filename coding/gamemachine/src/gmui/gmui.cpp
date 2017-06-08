#include "stdafx.h"
#include "gmui.h"

GMUIWindowBase::GMUIWindowBase()
{
	D(d);
	d->wnd = NULL;
	d->wndProc = ::DefWindowProc;
	d->subClassed = false;
}