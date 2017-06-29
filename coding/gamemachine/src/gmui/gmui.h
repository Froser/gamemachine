#ifndef __GMUI_H__
#define __GMUI_H__
#include "common.h"

#if _WINDOWS
#	include <windef.h>
#	include "uilib.h"

typedef DuiLib::CWindowWnd GMUIWindowBase;
typedef DuiLib::CPaintManagerUI GMUIPainter;
typedef HWND GMUIWindowHandle;
typedef WNDPROC GMUIWindowProc;
typedef HINSTANCE GMUIInstance;
typedef LPCTSTR GMUIStringPtr;
typedef LRESULT LongResult;
typedef WPARAM UintPtr;
typedef LPARAM LongPtr;

struct GMUIWindowAttributes
{
	HWND hwndParent;
	LPCTSTR pstrName;
	DWORD dwStyle;
	DWORD dwExStyle;
	RECT rc;
	HMENU hMenu;
	GMUIInstance instance;
};
#else
typedef void* GMUIWindowHandle;
typedef void* GMUIWindowProc;
typedef void* GMUIInstance;
typedef GMWchar* GMUIStringPtr;
typedef GMlong LongPtr;
typedef GMuint UintPtr;
typedef GMlong LongPtr;
#endif

BEGIN_NS
class GMUIWindow : public GMObject, private GMUIWindowBase
{
	typedef GMUIWindowBase Base;

public:
	static bool handleMessage();

public:
	GMUIWindowHandle getWindowHandle() const { return Base::GetHWND(); }
	operator GMUIWindowHandle() const { return getWindowHandle(); }

public:
	virtual GMUIWindowHandle create(const GMUIWindowAttributes& attrs);
	virtual GMRect getWindowRect();
	virtual GMRect getClientRect();
	virtual void swapBuffers() const {}
	virtual void centerWindow() { return Base::CenterWindow(); }
	virtual GMuint showModal() { return Base::ShowModal(); }
	virtual void showWindow(bool show = true, bool takeFocus = true) { Base::ShowWindow(show, takeFocus); }
	virtual void onFinalMessage(GMUIWindowHandle wndHandle) {}
	virtual void update() {};

private:
	virtual GMUIStringPtr getWindowClassName() const = 0;
	virtual GMuint getClassStyle() const { return 0; }

protected:
	virtual LongResult handleMessage(GMuint uMsg, UintPtr wParam, LongPtr lParam) { return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam); }

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
	virtual LongResult handleMessage(GMuint uMsg, UintPtr wParam, LongPtr lParam) override;

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
END_NS

#endif