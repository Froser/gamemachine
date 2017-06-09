#include "stdafx.h"
#include "gmui.h"

#if _WINDOWS
GMUIWindowBase::GMUIWindowBase()
{
	D(d);
	d->wnd = NULL;
	d->wndProc = ::DefWindowProc;
	d->subClassed = false;
}

#endif