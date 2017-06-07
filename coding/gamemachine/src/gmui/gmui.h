#ifndef __GMUI_H__
#define __GMUI_H__
#include "common.h"

#if _WINDOWS
#	include <windef.h>
#endif
BEGIN_NS

#if _WINDOWS
typedef HWND GMUIWindowHandle;
typedef WNDPROC GMUIWindowProc;
struct GMUIWindowAttributes
{
	HWND parent;
	LPCTSTR name;
	DWORD style;
	DWORD exStyle;
	RECT rc;
	HMENU menu;
};
#endif

GM_PRIVATE_OBJECT(GMUIWindowBase)
{
	GMUIWindowHandle wnd;
	GMUIWindowProc wndProc;
	bool subClass;
};

class GMUIWindowBase : public GMObject
{
	DECLARE_PRIVATE(GMUIWindowBase)

public:
	GMUIWindowBase();

	GMUIWindowHandle GetWindowHandle() const;
	operator GMUIWindowHandle() const;

	//bool RegisterWindowClass();
	//bool RegisterSuperclass();

	virtual HWND Create(const GMUIWindowAttributes& attrs) = 0;
	//HWND Create(HWND hwndParent, LPCTSTR pstrName, DWORD dwStyle, DWORD dwExStyle, int x = CW_USEDEFAULT, int y = CW_USEDEFAULT, int cx = CW_USEDEFAULT, int cy = CW_USEDEFAULT, HMENU hMenu = NULL);
	//HWND Subclass(HWND hWnd);
	//void Unsubclass();
	//void ShowWindow(bool bShow = true, bool bTakeFocus = true);
	//bool ShowModal();
	//void Close();
	//void CenterWindow();
	//void SetIcon(UINT nRes);
};
END_NS
#endif