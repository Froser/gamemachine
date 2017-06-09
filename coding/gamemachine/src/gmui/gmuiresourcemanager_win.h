#ifndef __GMUIRESOURCEMANAGER_WIN_H__
#define __GMUIRESOURCEMANAGER_WIN_H__
#if !_WINDOWS
#	error Only windows project can include this file
#endif

#include "common.h"
#include <wtypes.h>
#include <commctrl.h>

BEGIN_NS

#if _WINDOWS
typedef HWND GMUIWindowHandle;
typedef WNDPROC GMUIWindowProc;
typedef HINSTANCE GMUIInstance;
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
#endif

GM_PRIVATE_OBJECT(GMUIResourceManager)
{
	//
	HWND wndPaint;
	HDC dcPaint;
	HDC dcOffscreen;
	HBITMAP bmpOffscreen;
	HWND wndTooltip;
	TOOLINFO toolTip;
	bool showUpdateRect;

	//
	POINT lastMousePos;
	SIZE minWindow;
	SIZE initWindowSize;
	RECT sizeBox;
	SIZE roundCorner;
	RECT caption;
	UINT timerID;
	bool firstLayout;
	bool updateNeeded;
	bool focusNeeded;
	bool offscreenPaint;
	bool mouseTracking;
	bool mouseCapture;

	GMUIInstance instance;
};

// GMUIResourceManager 用于处理绘制流程
class GMUIResourceManager : public GMObject
{
	DECLARE_PRIVATE(GMUIResourceManager)

public:
	GMUIResourceManager();

public:
	static bool translateMessage(const LPMSG pMsg);
	static bool handleMessage();

public:
	void setResourceInstance(GMUIInstance hInstance);
	GMUIInstance getResourceInstance();
	void initWindow(GMUIWindowHandle handle);

private:
	GMUIWindowHandle getPaintWindow() const;
	bool preMessageHandler(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT& lRes);
};

END_NS
#endif