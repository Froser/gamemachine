#ifndef __GMUIRESOURCEMANAGER_H__
#define __GMUIRESOURCEMANAGER_H__
#if !_WINDOWS
#	error Only windows project can include this file
#endif

#include "common.h"
BEGIN_NS

#include <wtypes.h>
#include <commctrl.h>

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

	HINSTANCE instance;
};

class GMUIResourceManager : public GMSingleton<GMUIResourceManager>
{
	DECLARE_PRIVATE(GMUIResourceManager)
	DECLARE_SINGLETON(GMUIResourceManager);

public:
	GMUIResourceManager();

public:
	static void setResourceInstance(HINSTANCE hInstance);
	static HINSTANCE getResourceInstance();
};

END_NS
#endif