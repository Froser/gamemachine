#include "stdafx.h"
#include "gmui.h"
#include "foundation/gmprofile.h"
#include "foundation/gamemachine.h"

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

GMRect GMUIWindow::getClientRect()
{
	RECT rect;
	GetClientRect(getWindowHandle(), &rect);
	GMRect r = { (GMfloat)rect.left, (GMfloat)rect.top, (GMfloat)rect.right - rect.left, (GMfloat)rect.bottom - rect.top };
	return r;
}

//////////////////////////////////////////////////////////////////////////

void GMUIGUIWindow::hideWindow()
{
	::ShowWindow(getWindowHandle(), SW_HIDE);
}

bool GMUIGUIWindow::isWindowVisible()
{
	return !! ::IsWindowVisible(getWindowHandle());
}

LongResult GMUIGUIWindow::handleMessage(GMuint uMsg, UintPtr wParam, LongPtr lParam)
{
	D(d);
	LRESULT lRes = 0;
	BOOL bHandled = FALSE;
	switch (uMsg) {
	case WM_CREATE:        lRes = onCreate(uMsg, wParam, lParam, bHandled); break;
	case WM_CLOSE:         lRes = onClose(uMsg, wParam, lParam, bHandled); break;
	case WM_DESTROY:       lRes = onDestroy(uMsg, wParam, lParam, bHandled); break;
	case WM_NCACTIVATE:    lRes = onNcActivate(uMsg, wParam, lParam, bHandled); break;
	case WM_NCCALCSIZE:    lRes = onNcCalcSize(uMsg, wParam, lParam, bHandled); break;
	case WM_NCPAINT:       lRes = onNcPaint(uMsg, wParam, lParam, bHandled); break;
	case WM_NCHITTEST:     lRes = onNcHitTest(uMsg, wParam, lParam, bHandled); break;
	case WM_SIZE:          lRes = onSize(uMsg, wParam, lParam, bHandled); break;
	case WM_GETMINMAXINFO: lRes = onGetMinMaxInfo(uMsg, wParam, lParam, bHandled); break;
	case WM_SYSCOMMAND:    lRes = onSysCommand(uMsg, wParam, lParam, bHandled); break;
	default:
		bHandled = FALSE;
	}

	if (bHandled)
		return lRes;
	if (d->painter.MessageHandler(uMsg, wParam, lParam, lRes))
		return lRes;
	return Base::handleMessage(uMsg, wParam, lParam);
}
#endif