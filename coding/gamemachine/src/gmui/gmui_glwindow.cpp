#include "stdafx.h"
#include "gmui/gmui_glwindow.h"
#include "foundation/gamemachine.h"

#if _WINDOWS

GMUIGLWindow::GMUIGLWindow()
{
	D(d);
	d->depthBits = 24;
	d->stencilBits = 8;
}

GMUIGLWindow::~GMUIGLWindow()
{
	dispose();
}

GMUIWindowHandle GMUIGLWindow::create(const GMUIWindowAttributes& wndAttrs)
{
	D(d);

	GMUIWindowAttributes attrs = wndAttrs;
	attrs.dwExStyle |= WS_EX_CLIENTEDGE;
	attrs.dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_VISIBLE;

	// 在非全屏的时候才有效
	AdjustWindowRectEx(&attrs.rc, attrs.dwStyle, FALSE, attrs.dwExStyle);

	if (!Base::create(attrs))
	{
		dispose();
		DWORD s = GetLastError();
		gm_error("window created failed: %i", s);
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

	GMUIWindowHandle wnd = getWindowHandle();
	if (!(d->hDC = GetDC(wnd)))
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

	::ShowWindow(wnd, SW_SHOW);
	::SetForegroundWindow(wnd);
	::SetFocus(wnd);

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		ASSERT(!"glew init failed");
		return NULL;
	}

	return wnd;
}

void GMUIGLWindow::swapBuffers() const
{
	D(d);
	::SwapBuffers(d->hDC);
}

void GMUIGLWindow::dispose()
{
	D(d);
	GMUIWindowHandle wnd = getWindowHandle();
	if (d->hRC)
	{
		if (!wglMakeCurrent(0, 0))
			gm_error("release of DC and RC failed.");

		if (!wglDeleteContext(d->hRC))
			gm_error("release Rendering Context failed.");

		d->hRC = 0;
	}

	if (d->hDC && !ReleaseDC(wnd, d->hDC))
	{
		gm_error("release of Device Context failed.");
		d->hDC = 0;
	}

	if (wnd && !DestroyWindow(wnd))
	{
		gm_error("could not release hWnd");
		wnd = 0;
	}
}

#endif