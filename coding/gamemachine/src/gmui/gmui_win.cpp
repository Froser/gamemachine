#include "stdafx.h"
// This file is for windows only

#if _WINDOWS
#include "gmui.h"

GMUIWindow::GMUIWindow()
{
	D(d);
	d->wnd = NULL;
	d->wndProc = ::DefWindowProc;
	d->subClass = false;
}

GMUIWindowHandle GMUIWindow::GetWindowHandle() const
{
	D(d);
	return d->wnd;
}

GMUIWindow::operator GMUIWindowHandle() const
{
	D(d);
	return d->wnd;
}

#endif