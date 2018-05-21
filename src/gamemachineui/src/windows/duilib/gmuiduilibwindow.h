#ifndef __GMUIDUILIBWINDOW_H__
#define __GMUIDUILIBWINDOW_H__
#include <gmuicommon.h>
#include <gamemachine.h>
#include <uilib.h>
#include "../gmuidef.h"
#include "../gmuiguiwindow.h"

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUIDuiLibWindow)
{
	gm::IInput* input = nullptr;
	DuiLib::CPaintManagerUI painter;
};

class GMUIDuiLibWindow 
	: public gmui::GMUIGUIWindow
	, private DuiLib::CWindowWnd
{
	DECLARE_PRIVATE_AND_BASE(GMUIDuiLibWindow, GMUIGUIWindow)
	typedef DuiLib::CWindowWnd Implement;

public:
	operator gm::GMWindowHandle() const { return getWindowHandle(); }

	// 需要DuiLib子类实现的虚函数
public:
	virtual LPCTSTR getWindowClassName() const = 0;
	virtual UINT getClassStyle() const = 0;
	virtual void onFinalMessage(HWND hWnd) = 0;

	// 来自DuiLib的虚函数，进行一次转调
protected:
	virtual LPCTSTR GetWindowClassName() const override;
	virtual UINT GetClassStyle() const override;
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
	virtual void OnFinalMessage(HWND hWnd) override;

public:
	virtual gm::GMWindowHandle create(const gm::GMWindowAttributes& attrs) override;
	virtual gm::GMWindowHandle getWindowHandle() const override;
	virtual void showWindow() override { showWindowEx(); }

protected:
	virtual void showWindowEx(bool show = true, bool takeFocus = true) { Implement::ShowWindow(show, takeFocus); }
};

END_UI_NS
#endif