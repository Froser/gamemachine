#include "stdafx.h"
#include "gmuiduilibwindow.h"

gm::GMWindowHandle GMUIDuiLibWindow::create(const gm::GMWindowAttributes& attrs)
{
	return Implement::Create(attrs.hwndParent, attrs.windowName.toStdWString().c_str(), attrs.dwStyle, attrs.dwExStyle, attrs.rc, attrs.hMenu);
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

LRESULT GMUIDuiLibWindow::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	D(d);
	LRESULT lRes;
	bool bHandled = Base::wndProc(uMsg, wParam, lParam, &lRes);

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
