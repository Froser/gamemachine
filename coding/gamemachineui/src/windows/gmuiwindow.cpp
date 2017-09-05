#include "stdafx.h"
#include "gmui.h"
#include <gamemachine.h>

#if _WINDOWS

gm::GMWindowHandle GMUIWindow::create(const gm::GMWindowAttributes& wndAttrs)
{
	return Base::Create(wndAttrs.hwndParent, wndAttrs.pstrName, wndAttrs.dwStyle, wndAttrs.dwExStyle, wndAttrs.rc, wndAttrs.hMenu);
}

gm::GMRect GMUIWindow::getWindowRect()
{
	RECT rect;
	GetWindowRect(getWindowHandle(), &rect);
	gm::GMRect r = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
	return r;
}

gm::GMRect GMUIWindow::getClientRect()
{
	RECT rect;
	GetClientRect(getWindowHandle(), &rect);
	gm::GMRect r = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
	return r;
}

#endif