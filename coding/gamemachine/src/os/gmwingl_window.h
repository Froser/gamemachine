#ifndef __WINGL_WINDOW_H__
#define __WINGL_WINDOW_H__
#include "common.h"
BEGIN_NS

#ifdef _WINDOWS

GM_PRIVATE_OBJECT(GMWinGLWindow)
{
	char windowTitle[128];
	LONG left, top, width, height;
	GMint depthBits, stencilBits;
	HWND hWnd;
	HDC hDC;
	HGLRC hRC;
	MSG msg;
	HINSTANCE hInstance;
	// bool fullscreen;
};

// Windows下的Window类
class GMWinGLWindow : public GMObject, public IWindow
{
	DECLARE_PRIVATE(GMWinGLWindow)

public:
	GMWinGLWindow();
	~GMWinGLWindow();

public:
	virtual bool createWindow() override;
	virtual GMRect getWindowRect() override;
	virtual bool handleMessages() override;
	virtual void swapBuffers() override;
	virtual HWND hwnd() override;

private:
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

private:
	void dispose();
};

#endif

END_NS
#endif