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
	inline virtual GMUIStringPtr getWindowClassName() const = 0;
	inline virtual GMuint getClassStyle() const { return 0; }

	// From base:
protected:
	virtual LPCTSTR GetWindowClassName() const { return getWindowClassName(); }
	virtual UINT GetClassStyle() const { return getClassStyle(); }
};
END_NS

#endif