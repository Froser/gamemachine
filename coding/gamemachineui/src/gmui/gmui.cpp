#include "stdafx.h"
#include "gmui.h"
#include <gamemachine.h>
#include "gmui_glwindow.h"
#include "gmui_console.h"

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

//////////////////////////////////////////////////////////////////////////

void GMUIGUIWindow::hideWindow()
{
	::ShowWindow(getWindowHandle(), SW_HIDE);
}

bool GMUIGUIWindow::isWindowVisible()
{
	return !! ::IsWindowVisible(getWindowHandle());
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
	if (d->painter.MessageHandler(uMsg, wParam, lParam, lRes))
		return lRes;
	return Base::handleMessage(uMsg, wParam, lParam);
}

// factory
void GMUIFactory::createMainWindow(gm::GMInstance instance, OUT gm::IWindow** window)
{
	initEnvironment(instance);
	(*window) = new GMUIGLWindow();
}

void GMUIFactory::createConsoleWindow(gm::GMInstance instance, OUT gm::IDebugOutput** window)
{
	initEnvironment(instance);
	(*window) = new GMUIConsole();
}

void GMUIFactory::initEnvironment(gm::GMInstance instance)
{
	GMUIPainter::SetInstance(instance);
}

#endif