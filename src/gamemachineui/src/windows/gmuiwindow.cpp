#include "stdafx.h"
#include "gmui.h"
#include <gamemachine.h>
#include "gmuiwindow.h"
#include "gmuiinput.h"

#ifndef GetWindowOwner
#	define GetWindowOwner(hwnd) GetWindow(hwnd, GW_OWNER)
#endif

#ifndef GetWindowStyle
#	define GetWindowStyle(hwnd) ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#endif

namespace
{
	bool registerClass(const gm::GMWindowAttributes& wndAttrs, const gm::GMwchar* className)
	{
		WNDCLASS wc = { 0 };
		wc.style = 0;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hIcon = NULL;
		wc.lpfnWndProc = GMUIWindow::WndProc;
		wc.hInstance = wndAttrs.instance;
		wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = className;
		ATOM ret = ::RegisterClass(&wc);
		GM_ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
		return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
	}
}

GMUIWindow::~GMUIWindow()
{
	D(d);
	if (d->input)
		delete d->input;
}

gm::IInput* GMUIWindow::getInputMananger()
{
	D(d);
	if (!d->input)
		d->input = new GMUIInput(this);
	return d->input;
}

bool GMUIWindow::handleMessage()
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
			return false;
		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
	return true;
}

void GMUIWindow::update()
{
	D(d);
	if (d->input)
		d->input->update();
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

void GMUIWindow::centerWindow() 
{
	gm::GMWindowHandle hWnd = getWindowHandle();
	GM_ASSERT(::IsWindow(hWnd));
	GM_ASSERT((GetWindowStyle(hWnd)&WS_CHILD) == 0);
	RECT rcDlg = { 0 };
	::GetWindowRect(hWnd, &rcDlg);
	RECT rcArea = { 0 };
	RECT rcCenter = { 0 };
	HWND hWndParent = ::GetParent(hWnd);
	HWND hWndCenter = ::GetWindowOwner(hWnd);
	if (hWndCenter != NULL)
		hWnd = hWndCenter;

	// 处理多显示器模式下屏幕居中
	MONITORINFO oMonitor = {};
	oMonitor.cbSize = sizeof(oMonitor);
	::GetMonitorInfo(::MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST), &oMonitor);
	rcArea = oMonitor.rcWork;

	if (hWndCenter == NULL || IsIconic(hWndCenter))
		rcCenter = rcArea;
	else
		::GetWindowRect(hWndCenter, &rcCenter);

	int DlgWidth = rcDlg.right - rcDlg.left;
	int DlgHeight = rcDlg.bottom - rcDlg.top;

	// Find dialog's upper left based on rcCenter
	int xLeft = (rcCenter.left + rcCenter.right) / 2 - DlgWidth / 2;
	int yTop = (rcCenter.top + rcCenter.bottom) / 2 - DlgHeight / 2;

	// The dialog is outside the screen, move it inside
	if (xLeft < rcArea.left) xLeft = rcArea.left;
	else if (xLeft + DlgWidth > rcArea.right) xLeft = rcArea.right - DlgWidth;
	if (yTop < rcArea.top) yTop = rcArea.top;
	else if (yTop + DlgHeight > rcArea.bottom) yTop = rcArea.bottom - DlgHeight;
	::SetWindowPos(hWnd, NULL, xLeft, yTop, -1, -1, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}

bool GMUIWindow::isWindowActivate()
{
	return ::GetForegroundWindow() == getWindowHandle();
}

void GMUIWindow::setLockWindow(bool lock)
{
	if (lock)
		::SetCapture(getWindowHandle());
	else
		::ReleaseCapture();
}

bool GMUIWindow::createWindow(const gm::GMWindowAttributes& wndAttrs, const gm::GMwchar* className)
{
	D(d);
	registerClass(wndAttrs, className);
	gm::GMWindowHandle hwnd = ::CreateWindowEx(
		wndAttrs.dwExStyle,
		className,
		wndAttrs.windowName.toStdWString().c_str(),
		wndAttrs.dwStyle,
		wndAttrs.rc.left,
		wndAttrs.rc.top,
		wndAttrs.rc.right - wndAttrs.rc.left,
		wndAttrs.rc.bottom - wndAttrs.rc.top,
		wndAttrs.hwndParent,
		wndAttrs.hMenu,
		wndAttrs.instance,
		this);
	GM_ASSERT(hwnd);
	return !!hwnd;
}

LRESULT CALLBACK GMUIWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	GMUIWindow* pGMWindow = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pGMWindow = static_cast<GMUIWindow*>(lpcs->lpCreateParams);
		pGMWindow->setWindowHandle(hWnd);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LPARAM>(pGMWindow));
	}
	else
	{
		pGMWindow = reinterpret_cast<GMUIWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}

	if (pGMWindow)
	{
		LRESULT result = 0;
		if (pGMWindow->wndProc(uMsg, wParam, lParam, &result))
			return result;
	}
	return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

bool GMUIGameMachineWindowBase::wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes)
{
	D_BASE(db, GMUIWindow);

	bool handled = Base::wndProc(uMsg, wParam, lParam, lRes);
	if (handled)
		return handled;

	switch (uMsg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0L);
		break;
	case WM_SIZE:
	{
		gm::GameMachine::instance().postMessage({ gm::GameMachineMessageType::WindowSizeChanged });
		break;
	}
	case WM_MOUSEWHEEL:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordWheel(true, GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	}
	case WM_LBUTTONDOWN:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseDown(GMMouseButton_Left);
		break;
	}
	case WM_MOUSEMOVE:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseMove();
		break;
	}
	case WM_RBUTTONDOWN:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseDown(GMMouseButton_Right);
		break;
	}
	case WM_LBUTTONUP:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseUp(GMMouseButton_Left);
		break;
	}
	case WM_RBUTTONUP:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseUp(GMMouseButton_Right);
		break;
	}
	default:
		break;
	}

	*lRes = ::DefWindowProc(getWindowHandle(), uMsg, wParam, lParam);
	return true;
}

void GMUIGameMachineWindowBase::showWindow()
{
	D(d);
	gm::GMWindowHandle hwnd = getWindowHandle();
	GM_ASSERT(::IsWindow(hwnd));
	if (!::IsWindow(hwnd)) return;
	::ShowWindow(hwnd, SW_SHOWNORMAL);
}