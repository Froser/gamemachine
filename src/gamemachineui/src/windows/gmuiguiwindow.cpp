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

LongResult GMUIGUIWindow::handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam)
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
	case WM_SHOWWINDOW:    lRes = onShowWindow(uMsg, wParam, lParam, bHandled); break;
	default:
		bHandled = FALSE;
	}

	if (bHandled)
		return lRes;
	return Base::handleMessage(uMsg, wParam, lParam);
}
