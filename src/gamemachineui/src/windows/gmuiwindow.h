#ifndef __GMUIWINDOW_H__
#define __GMUIWINDOW_H__
#include <gmuicommon.h>
#include "gmuidef.h"
#include <gamemachine.h>

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUIWindow)
{
	gm::GMWindowHandle hwnd;
	gm::IInput* input = nullptr;
};

class GMUIWindow : public gm::GMObject, public gm::IWindow
{
	DECLARE_PRIVATE(GMUIWindow)

public:
	~GMUIWindow();

public:
	operator gm::GMWindowHandle() const { return getWindowHandle(); }

	// IWindow
public:
	virtual gm::IInput* getInputMananger() override;
	virtual void update() override;
	virtual gm::GMRect getWindowRect() override;
	virtual gm::GMRect getClientRect() override;
	virtual void centerWindow() override;
	virtual bool isWindowActivate() override;
	virtual void setLockWindow(bool lock) override;
	virtual bool event(const gm::GameMachineMessage& msg) override { return false; }
	virtual gm::GMWindowHandle getWindowHandle() const { D(d); return d->hwnd; }

	// 新虚方法
protected:
	virtual LRESULT wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) { bHandled = false; return 0; }

private:
	void setWindowHandle(gm::GMWindowHandle hwnd) { D(d); d->hwnd = hwnd; }

public:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

END_UI_NS

#endif