#ifndef __GMUI_H__
#define __GMUI_H__
#include "common.h"

#if _WINDOWS
#	include <windef.h>
#	include "uilib.h"
typedef DuiLib::CWindowWnd GMUIWindowBase;
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
	virtual void swapBuffers() const {}
	virtual void centerWindow() { return Base::CenterWindow(); }
	virtual void showWindow(bool show = true, bool takeFocus = true) { Base::ShowWindow(show, takeFocus); }

private:
	virtual GMUIStringPtr getWindowClassName() const = 0;
	virtual GMuint getClassStyle() const { return 0; }

protected:
	virtual LongResult handleMessage(GMuint uMsg, UintPtr wParam, LongPtr lParam) { return ::CallWindowProc(m_OldWndProc, m_hWnd, uMsg, wParam, lParam); }

	// From base:
protected:
	virtual LPCTSTR GetWindowClassName() const override { return getWindowClassName(); }
	virtual UINT GetClassStyle() const override { return getClassStyle(); }
	virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override { return handleMessage(uMsg, wParam, lParam); }
};
END_NS

#endif