#include "stdafx.h"
#include "gmengine/ui/gmwindow.h"
#include "foundation/platforms/windows/window/gminput.h"
#include "foundation/gamemachine.h"
#include "gmengine/ui/gmwidget.h"

#ifndef GetWindowOwner
#	define GetWindowOwner(hwnd) GetWindow(hwnd, GW_OWNER)
#endif

#ifndef GetWindowStyle
#	define GetWindowStyle(hwnd) ((DWORD)GetWindowLong(hwnd, GWL_STYLE))
#endif

namespace
{
	Map<HWND, GMWindow*> s_hwndMap;

	LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		GMWindow* pGMWindow = nullptr;
		if (uMsg == WM_NCCREATE)
		{
			LPCREATESTRUCT lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
			pGMWindow = static_cast<GMWindow*>(lpcs->lpCreateParams);
			pGMWindow->setWindowHandle(hWnd, true);
			s_hwndMap[hWnd] = pGMWindow;
		}
		else
		{
			auto iter = s_hwndMap.find(hWnd);
			if (iter != s_hwndMap.end())
				pGMWindow = (*iter).second;
		}

		if (pGMWindow)
		{
			LRESULT lRes = 0;
			GMSystemEvent* sysEvent = nullptr;
			GM.translateSystemEvent(uMsg, wParam, lParam, &sysEvent);
			GMScopedPtr<GMSystemEvent> guard(sysEvent);
			if (!pGMWindow->handleSystemEvent(sysEvent, lRes))
				return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	bool registerClass(const GMWindowDesc& wndAttrs, GMWindow* window, const GMwchar* className)
	{
		WNDCLASS wc = { 0 };
		wc.style = 0;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hIcon = NULL;
		wc.lpfnWndProc = window->getProcHandler();
		wc.hInstance = wndAttrs.instance;
		wc.hCursor = NULL;
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = className;
		ATOM ret = ::RegisterClass(&wc);
		GM_ASSERT(ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS);
		return ret != NULL || ::GetLastError() == ERROR_CLASS_ALREADY_EXISTS;
	}

	HCURSOR getCursor(GMCursorType cursorType)
	{
		constexpr GMuint32 sz = (GMuint32)GMCursorType::EndOfEnum;
		static HCURSOR cursors[sz] = { NULL };
		if (!cursors[(GMuint32)cursorType])
		{
			LPCWSTR cursor = NULL;
			switch (cursorType)
			{
			case GMCursorType::Arrow:
				cursor = IDC_ARROW;
				break;
			case GMCursorType::IBeam:
				cursor = IDC_IBEAM;
				break;
			case GMCursorType::Wait:
				cursor = IDC_WAIT;
				break;
			case GMCursorType::Cross:
				cursor = IDC_CROSS;
				break;
			case GMCursorType::UpArrow:
				cursor = IDC_UPARROW;
				break;
			case GMCursorType::Hand:
				cursor = IDC_HAND;
				break;
			default:
				GM_ASSERT(false);
				break;
			}

			cursors[(GMuint32)cursorType] = ::LoadCursor(NULL, cursor);
		}
		return cursors[(GMuint32)cursorType];
	}
}

IWindow* GMRenderContext::getWindow() const
{
	return const_cast<IWindow*>(window);
}

IGraphicEngine* GMRenderContext::getEngine() const
{
	return const_cast<IGraphicEngine*>(engine);
}

GMWindowProcHandler GMWindow::getProcHandler()
{
	return &WndProc;
}

GMWindow::GMWindow()
{
}

GMWindow::~GMWindow()
{
	D(d);
	if (d->ownedHandle)
	{
		GMWindowHandle hwnd = getWindowHandle();
		if (hwnd)
		{
			onWindowDestroyed();
			::DestroyWindow(hwnd);
		}
	}
}

IInput* GMWindow::getInputMananger()
{
	D(d);
	if (!d->input)
		d->input = gm_makeOwnedPtr<GMInput>(this);
	return d->input.get();
}

GMRect GMWindow::getWindowRect()
{
	RECT rect;
	GetWindowRect(getWindowHandle(), &rect);
	GMRect r = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
	return r;
}

GMRect GMWindow::getRenderRect()
{
	RECT rect;
	GetClientRect(getWindowHandle(), &rect);
	GMRect r = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
	return r;
}

void GMWindow::centerWindow()
{
	GMWindowHandle hWnd = getWindowHandle();
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

bool GMWindow::isWindowActivate()
{
	return ::GetForegroundWindow() == getWindowHandle();
}

void GMWindow::setWindowCapture(bool capture)
{
	if (capture)
		::SetCapture(getWindowHandle());
	else
		::ReleaseCapture();
}

GMWindowHandle GMWindow::create(const GMWindowDesc& desc)
{
	D(d);
	GMWindowHandle hwnd = NULL;
	GMWindowDesc attrs = desc;
	if (desc.createNewWindow)
	{
		// 在非全屏的时候才有效，计算出客户窗口（渲染窗口）大小
		::AdjustWindowRectEx(&attrs.rc, attrs.dwStyle, FALSE, attrs.dwExStyle);
		const GMwchar* className = getWindowClassName();
		registerClass(attrs, this, className);
		hwnd = ::CreateWindowEx(
			attrs.dwExStyle,
			className,
			attrs.windowName.toStdWString().c_str(),
			attrs.dwStyle,
			attrs.rc.left,
			attrs.rc.top,
			attrs.rc.right - attrs.rc.left,
			attrs.rc.bottom - attrs.rc.top,
			attrs.hwndParent,
			NULL,
			attrs.instance,
			this);
		GM_ASSERT(hwnd);
	}
	else
	{
		hwnd = attrs.existWindowHandle;
		setWindowHandle(attrs.existWindowHandle, false);
		const GMRect& rc = getRenderRect();
		attrs.rc = { rc.x, rc.y, rc.x + rc.width, rc.y + rc.height };
	}

	onWindowCreated(attrs);
	d->windowStates.renderRect = getRenderRect();
	d->windowStates.framebufferRect = d->windowStates.renderRect;
	return hwnd;
}

void GMWindow::changeCursor()
{
	D(d);
	::SetCursor(getCursor(d->cursor));
}

void GMWindow::showWindow()
{
	D(d);
	GMWindowHandle hwnd = getWindowHandle();
	GM_ASSERT(::IsWindow(hwnd));
	if (!::IsWindow(hwnd)) return;
	::ShowWindow(hwnd, SW_SHOWNORMAL);
}

void GMWindow::onWindowDestroyed()
{
	D(d);
	if (d->ownedHandle)
	{
		GMWindowHandle hwnd = getWindowHandle();
		GM_ASSERT(hwnd);
		auto s = s_hwndMap.erase(hwnd);
		GM_ASSERT(s > 0);
	}
}
