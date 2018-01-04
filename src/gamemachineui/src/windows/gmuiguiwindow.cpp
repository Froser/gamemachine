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

LRESULT GMUIGUIWindow::wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled)
{
	D(d);
	LRESULT lRes = 0;
	BOOL h = FALSE;
	switch (uMsg) {
	case WM_CREATE:        lRes = onCreate(uMsg, wParam, lParam, h); break;
	case WM_CLOSE:         lRes = onClose(uMsg, wParam, lParam, h); break;
	case WM_DESTROY:       lRes = onDestroy(uMsg, wParam, lParam, h); break;
	case WM_NCACTIVATE:    lRes = onNcActivate(uMsg, wParam, lParam, h); break;
	case WM_NCCALCSIZE:    lRes = onNcCalcSize(uMsg, wParam, lParam, h); break;
	case WM_NCPAINT:       lRes = onNcPaint(uMsg, wParam, lParam, h); break;
	case WM_NCHITTEST:     lRes = onNcHitTest(uMsg, wParam, lParam, h); break;
	case WM_SIZE:          lRes = onSize(uMsg, wParam, lParam, h); break;
	case WM_GETMINMAXINFO: lRes = onGetMinMaxInfo(uMsg, wParam, lParam, h); break;
	case WM_SYSCOMMAND:    lRes = onSysCommand(uMsg, wParam, lParam, h); break;
	case WM_SHOWWINDOW:    lRes = onShowWindow(uMsg, wParam, lParam, h); break;
	default:
		h = FALSE;
	}

	bHandled = !!h;
	if (h)
		return lRes;
	return Base::wndProc(uMsg, wParam, lParam, bHandled);
}
