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

	//! 处理新一轮消息循环。
	/*!
	  此方法由GameMachine调用。处理一轮消息循环。<br>
	  在Windows下，消息循环的行为应该是GetMessage（或PeekMessage）、TranslateMessage和DispatchMessage。<br>
	  消息循环的处理函数为WndProc()，在本方法被调用后，Windows将通过DispatchMessage()调用WndProc()。<br>
	  \return 是否应该继续消息循环。如果返回false，则程序退出。
	  \sa WndProc()
	*/
	virtual bool handleMessage() override;
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
	virtual bool wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool createWindow(const gm::GMWindowAttributes& wndAttrs, const gm::GMwchar* className);

private:
	void setWindowHandle(gm::GMWindowHandle hwnd) { D(d); d->hwnd = hwnd; }

public:
	//! 窗口处理函数。
	/*!
	  在创建Windows窗口类时，此函数指针被传入。在初始化时，此函数将窗口句柄赋值给对应的GMUIWindow，并将GMUIWindow的实例与句柄关联。<br>
	  接着，在消息循环进行时，通过句柄获取相应的GMUIWindow对象，调用其wndProc虚方法。<br>
	  \sa wndProc(), handleMessage()
	*/
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

class GMUIGameMachineWindowBase : public GMUIWindow
{
	typedef GMUIWindow Base;

protected:
	bool wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes);

public:
	virtual void showWindow() override;
};

END_UI_NS

#endif