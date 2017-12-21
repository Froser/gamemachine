#include "stdafx.h"
#include "gmui.h"
#include <gamemachine.h>
#include "gmuiinput.h"

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
		d->input = new GMInput(this);
	return d->input;
}

void GMUIWindow::update()
{
	D(d);
	if (d->input)
		d->input->update();
}

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

bool GMUIWindow::isWindowActivate()
{
	return ::GetForegroundWindow() == getWindowHandle();
}

LongResult GMUIWindow::handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam)
{
	return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam);
}

void GMUIWindow::setLockWindow(bool lock)
{
	if (lock)
		::SetCapture(getWindowHandle());
	else
		::ReleaseCapture();
}