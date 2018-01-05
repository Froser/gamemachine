#include "stdafx.h"
#include "gmuiguiwindow.h"

void GMUIGUIWindow::hideWindow()
{
	::ShowWindow(getWindowHandle(), SW_HIDE);
}

bool GMUIGUIWindow::isWindowVisible()
{
	return !!::IsWindowVisible(getWindowHandle());
}

void GMUIGUIWindow::refreshWindow()
{
	RECT rc;
	::GetClientRect(getWindowHandle(), &rc);
	::InvalidateRect(getWindowHandle(), &rc, TRUE);
}

bool GMUIGUIWindow::wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes)
{
	D(d);
	bool handled = false;
	switch (uMsg) {
	case WM_CREATE:        handled = onCreate(uMsg, wParam, lParam, lRes); break;
	case WM_CLOSE:         handled = onClose(uMsg, wParam, lParam, lRes); break;
	case WM_DESTROY:       handled = onDestroy(uMsg, wParam, lParam, lRes); break;
	case WM_NCACTIVATE:    handled = onNcActivate(uMsg, wParam, lParam, lRes); break;
	case WM_NCCALCSIZE:    handled = onNcCalcSize(uMsg, wParam, lParam, lRes); break;
	case WM_NCPAINT:       handled = onNcPaint(uMsg, wParam, lParam, lRes); break;
	case WM_NCHITTEST:     handled = onNcHitTest(uMsg, wParam, lParam, lRes); break;
	case WM_SIZE:          handled = onSize(uMsg, wParam, lParam, lRes); break;
	case WM_GETMINMAXINFO: handled = onGetMinMaxInfo(uMsg, wParam, lParam, lRes); break;
	case WM_SYSCOMMAND:    handled = onSysCommand(uMsg, wParam, lParam, lRes); break;
	case WM_SHOWWINDOW:    handled = onShowWindow(uMsg, wParam, lParam, lRes); break;
	default:
		handled = false;
	}

	if (handled)
		return handled;
	return Base::wndProc(uMsg, wParam, lParam, lRes);
}
