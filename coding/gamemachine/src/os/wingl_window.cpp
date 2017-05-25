#include "stdafx.h"
#include "wingl_window.h"

#ifdef _WINDOWS

const char* CLASSNAME = "GameMachine Window";

WinGLWindow::WinGLWindow()
{
	D(d);
	strcpy_s(d->windowTitle, "GM");
	d->depthBits = 24;
	d->stencilBits = 8;
	d->width = 700;
	d->height = 400;

	GMint screenWidth = GetSystemMetrics(SM_CXSCREEN),
		screenHeight = GetSystemMetrics(SM_CYSCREEN);
	d->left = (screenWidth - d->width) * .5f;
	d->top = (screenHeight - d->height) * .5f;
}

WinGLWindow::~WinGLWindow()
{
	dispose();
}

bool WinGLWindow::createWindow()
{
	D(d);

	WNDCLASS wc;
	DWORD dwExStyle;
	DWORD dwStyle;

	RECT windowRect;
	windowRect.left = d->left;
	windowRect.right = d->left + d->width;
	windowRect.top = d->top;
	windowRect.bottom = d->top + d->height;

	d->hInstance = GetModuleHandle(NULL);
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = d->hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = CLASSNAME;

	if (!RegisterClass(&wc))
	{
		gm_error("failed to register the window class");
		return false;
	}

	dwExStyle = WS_EX_CLIENTEDGE;
	dwStyle = WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_VISIBLE;

	// 在非全屏的时候才有效
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	if (!(d->hWnd = CreateWindowEx(dwExStyle,
		CLASSNAME,							//class name
		d->windowTitle,						//window title
		WS_CLIPSIBLINGS |					//required style
		WS_CLIPCHILDREN |					//required style
		dwStyle,							//Selected style
		windowRect.left, windowRect.top,	//window position
		windowRect.right - windowRect.left,	//calculate adjusted width
		windowRect.bottom - windowRect.top,	//calculate adjusted height
		NULL,								// no parent window
		NULL,								//No Menu
		d->hInstance,						//Instance
		NULL)))								//Dont pass anything to WM_CREATE
	{
		dispose();
		gm_error("window created failed.");
		return false;
	}

	const GMbyte colorDepth = 32, alphaBits = 8;
	GMuint pixelFormat;
	static PIXELFORMATDESCRIPTOR pfd =						//pfd tells windows how we want things to be
	{
		sizeof(PIXELFORMATDESCRIPTOR),						//size of Pixel format descriptor
		1,													//Version Number
		PFD_DRAW_TO_WINDOW |								//must support window
		PFD_SUPPORT_OPENGL |								//must support opengl
		PFD_DOUBLEBUFFER,									//must support double buffer
		PFD_TYPE_RGBA,										//request RGBA format
		colorDepth,											//select color depth
		0, 0, 0, 0, 0, 0,									//color bits ignored
		alphaBits,											//alpha buffer bits
		0,													//shift bit ignored
		0,													//no accumulation buffer
		0, 0, 0, 0,											//accumulation bits ignored
		d->depthBits,										//z buffer bits
		d->stencilBits,										//stencil buffer bits
		0,													//no auxiliary buffer
		PFD_MAIN_PLANE,										//main drawing layer
		0,													//reserved
		0, 0, 0												//layer masks ignored
	};

	if (!(d->hDC = GetDC(d->hWnd)))
	{
		dispose();
		gm_error("can't Create a GL Device context.");
		return false;
	}

	if (!(pixelFormat = ChoosePixelFormat(d->hDC, &pfd)))	//found a matching pixel format?
	{														//if not
		dispose();
		gm_error("can't find a Suitable PixelFormat.");
		return false;
	}

	if (!SetPixelFormat(d->hDC, pixelFormat, &pfd))			//are we able to set pixel format?
	{														//if not
		dispose();
		gm_error("can't set the pixelformat.");
		return false;
	}

	if (!(d->hRC = wglCreateContext(d->hDC)))
	{
		dispose();
		gm_error("can't create a GL rendering context.");
		return false;
	}

	if (!wglMakeCurrent(d->hDC, d->hRC))
	{
		dispose();
		gm_error("can't activate the GL rendering context.");
		return false;
	}

	ShowWindow(d->hWnd, SW_SHOW);
	SetForegroundWindow(d->hWnd);
	SetFocus(d->hWnd);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		ASSERT(!"glew init failed");
		return false;
	}

	return true;
}

LRESULT CALLBACK WinGLWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		return 0;
		break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

GMRect WinGLWindow::getWindowRect()
{
	D(d);
	RECT rect;
	GetWindowRect(d->hWnd, &rect);
	GMRect r = { rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top };
	return r;
}

bool WinGLWindow::handleMessages()
{
	D(d);
	while (PeekMessage(&d->msg, NULL, 0, 0, PM_REMOVE))
	{
		if (d->msg.message == WM_QUIT)
			return false;

		TranslateMessage(&d->msg);
		DispatchMessage(&d->msg);
	}
	return true;
}

void WinGLWindow::swapBuffers()
{
	D(d);
	::SwapBuffers(d->hDC);
}

HWND WinGLWindow::hwnd()
{
	D(d);
	return d->hWnd;
}

void WinGLWindow::dispose()
{
	D(d);
	if (d->hRC)
	{
		if (!wglMakeCurrent(0, 0))
			gm_error("release of DC and RC failed.");

		if (!wglDeleteContext(d->hRC))
			gm_error("release Rendering Context failed.");

		d->hRC = 0;
	}

	if (d->hDC && !ReleaseDC(d->hWnd, d->hDC))
	{
		gm_error("release of Device Context failed.");
		d->hDC = 0;
	}

	if (d->hWnd && !DestroyWindow(d->hWnd))
	{
		gm_error("could not release hWnd");
		d->hWnd = 0;
	}

	if (!UnregisterClass(CLASSNAME, d->hInstance))
	{
		gm_error("could not unregister class.");
		d->hInstance = 0;
	}
}

#endif