#include "stdafx.h"
// This file is for windows only

#if _WINDOWS
#include "gmui.h"
#include "gmuiresourcemanager_win.h"
#include <windowsx.h>
#include "foundation/gamemachine.h"

UINT GMUIWindow::getClassStyle() const
{
	return 0;
}

LPCTSTR GMUIWindow::getSuperClassName() const
{
	return NULL;
}

GMUIWindowHandle GMUIWindow::create(const GMUIWindowAttributes& wndAttrs)
{
	D(d);
	d->ui.setResourceInstance(wndAttrs.instance);

	if (getSuperClassName() && !registerSuperclass())
		return NULL;
	if (!getSuperClassName() && !registerWindowClass())
		return NULL;

	d->wnd = ::CreateWindowEx(
		wndAttrs.dwExStyle,
		getWindowClassName(), 
		wndAttrs.pstrName,
		wndAttrs.dwStyle,
		wndAttrs.rc.left,
		wndAttrs.rc.top,
		wndAttrs.rc.right - wndAttrs.rc.left,
		wndAttrs.rc.bottom - wndAttrs.rc.top,
		wndAttrs.hwndParent,
		wndAttrs.hMenu,
		d->ui.getResourceInstance(),
		this
	);
	ASSERT(d->wnd);
	return d->wnd;
}

HWND GMUIWindow::subclass(HWND hWnd)
{
	D(d);
	ASSERT(::IsWindow(hWnd));
	ASSERT(d->wnd == NULL);
	d->wndProc = SubclassWindow(hWnd, __wndProc);
	if (d->wndProc == NULL) return NULL;
	d->subClassed = true;
	d->wnd = hWnd;
	return d->wnd;
}

void GMUIWindow::unsubclass()
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	if (!::IsWindow(d->wnd)) return;
	if (!d->subClassed) return;
	SubclassWindow(d->wnd, d->wndProc);
	d->wndProc = ::DefWindowProc;
	d->subClassed = false;
}

void GMUIWindow::showWindow(bool bShow /*= true*/, bool bTakeFocus /*= false*/)
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	if (!::IsWindow(d->wnd))
		return;
	::ShowWindow(d->wnd, bShow ? (bTakeFocus ? SW_SHOWNORMAL : SW_SHOWNOACTIVATE) : SW_HIDE);
}

bool GMUIWindow::showModal()
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	HWND hWndParent = GetWindowOwner(d->wnd);
	MSG msg = { 0 };
	while (::IsWindow(d->wnd) && ::GetMessage(&msg, NULL, 0, 0))
	{
		if (msg.hwnd == hWndParent) {
			if ((msg.message >= WM_MOUSEFIRST && msg.message <= WM_MOUSELAST))
				continue;
			if (msg.message >= WM_KEYFIRST && msg.message <= WM_KEYLAST)
				continue;
			if (msg.message == WM_SETCURSOR)
				continue;
		}
		if (!GMUIResourceManager::translateMessage(&msg))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		if (msg.message == WM_QUIT)
			break;
	}
	if (msg.message == WM_QUIT)
		::PostQuitMessage(msg.wParam);
	return true;
}

void GMUIWindow::close()
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	if (!::IsWindow(d->wnd)) return;
	postMessage(WM_CLOSE);
}

void GMUIWindow::centerWindow()
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	ASSERT((GetWindowStyle(d->wnd)&WS_CHILD) == 0);
	RECT rcDlg = { 0 };
	::GetWindowRect(d->wnd, &rcDlg);
	RECT rcArea = { 0 };
	RECT rcCenter = { 0 };
	HWND hWndParent = ::GetParent(d->wnd);
	HWND hWndCenter = ::GetWindowOwner(d->wnd);
	::SystemParametersInfo(SPI_GETWORKAREA, NULL, &rcArea, NULL);
	if (hWndCenter == NULL) rcCenter = rcArea; else ::GetWindowRect(hWndCenter, &rcCenter);

	GMint DlgWidth = rcDlg.right - rcDlg.left;
	GMint DlgHeight = rcDlg.bottom - rcDlg.top;

	// Find dialog's upper left based on rcCenter
	GMint xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	GMint yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	// The dialog is outside the screen, move it inside
	if (xLeft < rcArea.left)
		xLeft = rcArea.left;
	else if (xLeft + DlgWidth > rcArea.right)
		xLeft = rcArea.right - DlgWidth;
	if (yTop < rcArea.top)
		yTop = rcArea.top;
	else if (yTop + DlgHeight > rcArea.bottom)
		yTop = rcArea.bottom - DlgHeight;
	::SetWindowPos(d->wnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

GMRect GMUIWindow::getWindowRect()
{
	D(d);
	RECT rect;
	GetWindowRect(d->wnd, &rect);
	GMRect r = { (GMfloat)rect.left, (GMfloat)rect.top, (GMfloat)rect.right - rect.left, (GMfloat)rect.bottom - rect.top };
	return r;
}

void GMUIWindow::setIcon(UINT nRes)
{
	D(d);
	HICON hIcon = (HICON)::LoadImage(d->ui.getResourceInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON), LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(d->wnd, WM_SETICON, (WPARAM)TRUE, (LPARAM)hIcon);
	hIcon = (HICON)::LoadImage(d->ui.getResourceInstance(), MAKEINTRESOURCE(nRes), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);
	ASSERT(hIcon);
	::SendMessage(d->wnd, WM_SETICON, (WPARAM)FALSE, (LPARAM)hIcon);
}

bool GMUIWindow::registerWindowClass()
{
	D(d);
	WNDCLASS wc = { 0 };
	wc.style = getClassStyle();
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hIcon = ::LoadIcon(NULL, IDI_WINLOGO);
	wc.lpfnWndProc = GMUIWindow::__wndProc;
	wc.hInstance = d->ui.getResourceInstance();
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = getWindowClassName();
	ATOM ret = ::RegisterClass(&wc);
	ASSERT(ret || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
	return ret || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

bool GMUIWindow::registerSuperclass()
{
	D(d);
	// Get the class information from an existing
	// window so we can subclass it later on...
	WNDCLASSEX wc = { 0 };
	wc.cbSize = sizeof(WNDCLASSEX);
	if (!::GetClassInfoEx(NULL, getSuperClassName(), &wc)) {
		if (!::GetClassInfoEx(d->ui.getResourceInstance(), getSuperClassName(), &wc)) {
			ASSERT(!"Unable to locate window class");
			return NULL;
		}
	}
	d->wndProc = wc.lpfnWndProc;
	wc.lpfnWndProc = GMUIWindow::__controlProc;
	wc.hInstance = d->ui.getResourceInstance();
	wc.lpszClassName = getWindowClassName();
	ATOM ret = ::RegisterClassEx(&wc);
	ASSERT(ret || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
	return ret || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
}

LRESULT CALLBACK GMUIWindow::__wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	GMUIWindow* pThis = NULL;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<GMUIWindow*>(lpcs->lpCreateParams);
		D_OF(d, pThis);
		d->wnd = hWnd;
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pThis));
	}
	else
	{
		pThis = reinterpret_cast<GMUIWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
		if (uMsg == WM_NCDESTROY && pThis)
		{
			D_OF(d, pThis);
			LRESULT lRes = ::CallWindowProc(d->wndProc, hWnd, uMsg, wParam, lParam);
			::SetWindowLongPtr(d->wnd, GWLP_USERDATA, 0L);
			if (d->subClassed)
				pThis->unsubclass();
			d->wnd = NULL;
			pThis->onFinalMessage(hWnd);
			return lRes;
		}
	}
	if (pThis)
		return pThis->handleMessage(uMsg, wParam, lParam);
	else
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT CALLBACK GMUIWindow::__controlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	GMUIWindow* pThis = NULL;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = static_cast<GMUIWindow*>(lpcs->lpCreateParams);
		D_OF(d, pThis);
		::SetProp(hWnd, _L("WndX"), (HANDLE)pThis);
		d->wnd = hWnd;
	}
	else {
		pThis = reinterpret_cast<GMUIWindow*>(::GetProp(hWnd, _L("WndX")));
		if (uMsg == WM_NCDESTROY && pThis)
		{
			D_OF(d, pThis);
			LRESULT lRes = ::CallWindowProc(d->wndProc, hWnd, uMsg, wParam, lParam);
			if (d->subClassed)
				pThis->unsubclass();
			::SetProp(hWnd, _L("WndX"), NULL);
			d->wnd = NULL;
			pThis->onFinalMessage(hWnd);
			return lRes;
		}
	}
	if (pThis)
		return pThis->handleMessage(uMsg, wParam, lParam);
	else
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

LRESULT GMUIWindow::sendMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	return ::SendMessage(d->wnd, uMsg, wParam, lParam);
}

LRESULT GMUIWindow::postMessage(UINT uMsg, WPARAM wParam /*= 0*/, LPARAM lParam /*= 0*/)
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	return ::PostMessage(d->wnd, uMsg, wParam, lParam);
}

void GMUIWindow::resizeClient(GMint cx /*= -1*/, GMint cy /*= -1*/)
{
	D(d);
	ASSERT(::IsWindow(d->wnd));
	RECT rc = { 0 };
	if (!::GetClientRect(d->wnd, &rc))
		return;
	if (cx != -1)
		rc.right = cx;
	if (cy != -1)
		rc.bottom = cy;
	if (!::AdjustWindowRectEx(&rc, GetWindowStyle(d->wnd), (!(GetWindowStyle(d->wnd) & WS_CHILD) && (::GetMenu(d->wnd))), GetWindowExStyle(d->wnd)))
		return;
	UINT uFlags = SWP_NOZORDER | SWP_NOMOVE;
	::SetWindowPos(d->wnd, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, uFlags);
}

LRESULT GMUIWindow::handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	D(d);
	switch (uMsg)
	{
	case WM_CREATE:
		LONG styleValue = ::GetWindowLong(*this, GWL_STYLE);
		styleValue &= ~WS_CAPTION;
		::SetWindowLong(*this, GWL_STYLE, styleValue | WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
		RECT rcClient;
		::GetClientRect(*this, &rcClient);
		::SetWindowPos(*this, NULL, rcClient.left, rcClient.top, rcClient.right - rcClient.left, \
			rcClient.bottom - rcClient.top, SWP_FRAMECHANGED);

		d->ui.initWindow(d->wnd);
		break;
	}

	return ::CallWindowProc(d->wndProc, d->wnd, uMsg, wParam, lParam);
}

void GMUIWindow::onFinalMessage(HWND /*hWnd*/)
{
}


#endif