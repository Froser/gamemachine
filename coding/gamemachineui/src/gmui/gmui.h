#ifndef __GMUI_H__
#define __GMUI_H__
#include "../../common.h"
#include <gamemachine.h>

#if _WINDOWS
#	include <windef.h>
#	include "uilib.h"

typedef DuiLib::CWindowWnd GMUIWindowBase;
typedef DuiLib::CPaintManagerUI GMUIPainter;
typedef WNDPROC GMUIWindowProc;
typedef HINSTANCE GMUIInstance;
typedef LPCTSTR GMUIStringPtr;
typedef LRESULT LongResult;
typedef WPARAM UintPtr;
typedef LPARAM LongPtr;

#else
typedef void* gm::GMWindowHandle;
typedef void* GMUIWindowProc;
typedef void* GMUIInstance;
typedef gm::GMWchar* GMUIStringPtr;
typedef gm::GMlong LongPtr;
typedef gm::GMuint UintPtr;
typedef gm::GMlong LongPtr;
#endif

BEGIN_UI_NS
class GMUIWindow : public gm::GMObject, public gm::IWindow, private GMUIWindowBase
{
	typedef GMUIWindowBase Base;

public:
	operator gm::GMWindowHandle() const { return getWindowHandle(); }

	// IWindow
public:
	virtual void update() override {} ;
	virtual void swapBuffers() const override {};
	virtual gm::GMWindowHandle create(const gm::GMWindowAttributes& attrs) override;
	virtual gm::GMRect getWindowRect() override;
	virtual gm::GMRect getClientRect() override;
	virtual void showWindow() override { showWindowEx(); }
	virtual void centerWindow() override { return Base::CenterWindow(); }
	gm::GMWindowHandle getWindowHandle() const override { return Base::GetHWND(); }
	virtual bool handleMessage() override { return DuiLib::CPaintManagerUI::HandleMessage(); }

public:
	virtual gm::GMuint showModal() { return Base::ShowModal(); }
	virtual void showWindowEx(bool show = true, bool takeFocus = true) { Base::ShowWindow(show, takeFocus); }
	virtual void onFinalMessage(gm::GMWindowHandle wndHandle) {}

private:
	virtual GMUIStringPtr getWindowClassName() const = 0;
	virtual gm::GMuint getClassStyle() const { return 0; }

protected:
	virtual LongResult handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam) { return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam); }

#if _WINDOWS
	// From base:
protected:
	virtual LPCTSTR GetWindowClassName() const override { return getWindowClassName(); }
	virtual UINT GetClassStyle() const override { return getClassStyle(); }
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override { return handleMessage(uMsg, wParam, lParam); }
	virtual void OnFinalMessage(HWND hWnd) override { onFinalMessage(hWnd); }
#endif
};

#if _WINDOWS

// 一个标准的Windows GUI窗体
GM_PRIVATE_OBJECT(GMUIGUIWindow)
{
	GMUIPainter painter;
};

class GMUIGUIWindow : public GMUIWindow
{
	DECLARE_PRIVATE(GMUIGUIWindow);
	typedef GMUIWindow Base;

public:
	void hideWindow();
	bool isWindowVisible();
	void refreshWindow();

public:
	virtual LongResult handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam) override;

protected:
	virtual LongResult onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
};

#endif

//Factory
class GMUIFactory
{
public:
	static void createMainWindow(gm::GMInstance instance, OUT gm::IWindow** window);
	static void createConsoleWindow(gm::GMInstance instance, OUT gm::IDebugOutput** window);

private:
	static void initEnvironment(gm::GMInstance instance);
};

END_UI_NS

#endif