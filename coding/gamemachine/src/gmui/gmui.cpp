#include "stdafx.h"
#include "gmui.h"
#include "foundation/gmprofile.h"

#if _WINDOWS
bool GMUIWindow::handleMessage()
{
	GM_PROFILE(handleMessage);
	return DuiLib::CPaintManagerUI::HandleMessage();
}

GMUIWindowHandle GMUIWindow::create(const GMUIWindowAttributes& wndAttrs)
{
	return Base::Create(wndAttrs.hwndParent, wndAttrs.pstrName, wndAttrs.dwStyle, wndAttrs.dwExStyle, wndAttrs.rc, wndAttrs.hMenu);
}

GMRect GMUIWindow::getWindowRect()
{
	RECT rect;
	GetWindowRect(getWindowHandle(), &rect);
	GMRect r = { (GMfloat)rect.left, (GMfloat)rect.top, (GMfloat)rect.right - rect.left, (GMfloat)rect.bottom - rect.top };
	return r;
}
#endif