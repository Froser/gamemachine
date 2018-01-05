#include "stdafx.h"
#include "gmuiduilibwindow.h"

gm::GMWindowHandle GMUIDuiLibWindow::create(const gm::GMWindowAttributes& attrs)
{
	return Implement::Create(attrs.hwndParent, attrs.pstrName, attrs.dwStyle, attrs.dwExStyle, attrs.rc, attrs.hMenu);
}

gm::GMWindowHandle GMUIDuiLibWindow::getWindowHandle() const
{
	return Implement::GetHWND();
}

LPCTSTR GMUIDuiLibWindow::GetWindowClassName() const 
{
	return getWindowClassName();
}

UINT GMUIDuiLibWindow::GetClassStyle() const
{
	return getClassStyle();
}

bool GMUIDuiLibWindow::handleMessage()
{
	return DuiLib::CPaintManagerUI::HandleMessage();
}

LRESULT GMUIDuiLibWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	D(d);
	bool bHandled = false;
	LRESULT lRes = Base::wndProc(uMsg, wParam, lParam, bHandled);

	if (!bHandled)
	{
		if (d->painter.MessageHandler(uMsg, wParam, lParam, lRes))
			return lRes;
		return ::DefWindowProc(getWindowHandle(), uMsg, wParam, lParam);;
	}
	return lRes;
}

void GMUIDuiLibWindow::OnFinalMessage(HWND hWnd)
{
	onFinalMessage(hWnd);
}
