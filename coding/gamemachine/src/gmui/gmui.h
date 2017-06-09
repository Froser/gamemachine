#ifndef __GMUI_H__
#define __GMUI_H__
#include "common.h"
#include "gmuiresourcemanager.h"

#if _WINDOWS
#	include <windef.h>
#endif

BEGIN_NS
GM_PRIVATE_OBJECT(GMUIWindowBase)
{
	GMUIResourceManager ui;
	GMUIWindowHandle wnd;
	GMUIWindowProc wndProc;
	bool subClassed;
};

class GMUIWindowBase : public GMObject
{
	DECLARE_PRIVATE(GMUIWindowBase)

public:
	GMUIWindowBase();

public:
	GMUIWindowHandle getWindowHandle() const { D(d); return d->wnd; }
	operator GMUIWindowHandle() const { return getWindowHandle(); }

public:
	virtual GMUIWindowHandle create(const GMUIWindowAttributes& attrs) = 0;
	virtual void centerWindow() = 0;
	virtual void showWindow(bool bShow = true, bool bTakeFocus = true) = 0;
	virtual GMRect getWindowRect() = 0;
	virtual void swapBuffers() const = 0;
};

#if _WINDOWS
class GMUIWindow : public GMUIWindowBase
{
public:
	virtual GMUIWindowHandle create(const GMUIWindowAttributes& wndAttrs) override;
	virtual void centerWindow() override;
	virtual GMRect getWindowRect() override;
	virtual void swapBuffers() const override {}

public:
	HWND subclass(HWND hWnd);
	void unsubclass();
	void showWindow(bool bShow = true, bool bTakeFocus = true);
	bool showModal();
	void close();
	void setIcon(UINT nRes);

	LRESULT sendMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
	LRESULT postMessage(UINT uMsg, WPARAM wParam = 0, LPARAM lParam = 0L);
	void resizeClient(int cx = -1, int cy = -1);

private:
	bool registerWindowClass();
	bool registerSuperclass();

protected:
	virtual LPCTSTR getWindowClassName() const = 0;
	virtual LPCTSTR getSuperClassName() const;
	virtual UINT getClassStyle() const;

	virtual LRESULT handleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void onFinalMessage(HWND hWnd);

	static LRESULT CALLBACK __wndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK __controlProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};
#endif
END_NS
#endif