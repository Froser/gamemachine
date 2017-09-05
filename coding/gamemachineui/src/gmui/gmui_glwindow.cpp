#include "stdafx.h"
#include "gmui_glwindow.h"
#include <gamemachine.h>

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

gm::GMWindowHandle GMUIGLWindow::create(const gm::GMWindowAttributes& wndAttrs)
{
	D(d);

	gm::GMWindowAttributes attrs = wndAttrs;
	attrs.dwExStyle |= WS_EX_CLIENTEDGE;
	attrs.dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_BORDER | WS_CAPTION | WS_VISIBLE;

	// 在非全屏的时候才有效
	AdjustWindowRectEx(&attrs.rc, attrs.dwStyle, FALSE, attrs.dwExStyle);

	if (!Base::create(attrs))
	{
		dispose();
		DWORD s = GetLastError();
		gm_error(_L("window created failed: %i"), s);
		return false;
	}

	const gm::GMbyte colorDepth = 32, alphaBits = 8;
	gm::GMuint pixelFormat;
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

	gm::GMWindowHandle wnd = getWindowHandle();
	if (!(d->hDC = GetDC(wnd)))
	{
		dispose();
		gm_error(_L("can't Create a GL Device context."));
		return false;
	}

	if (!(pixelFormat = ChoosePixelFormat(d->hDC, &pfd)))	//found a matching pixel format?
	{														//if not
		dispose();
		gm_error(_L("can't find a Suitable PixelFormat."));
		return false;
	}

	if (!SetPixelFormat(d->hDC, pixelFormat, &pfd))			//are we able to set pixel format?
	{														//if not
		dispose();
		gm_error(_L("can't set the pixelformat."));
		return false;
	}

	if (!(d->hRC = wglCreateContext(d->hDC)))
	{
		dispose();
		gm_error(_L("can't create a GL rendering context."));
		return false;
	}

	if (!wglMakeCurrent(d->hDC, d->hRC))
	{
		dispose();
		gm_error(_L("can't activate the GL rendering context."));
		return false;
	}

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

LongResult GMUIGLWindow::handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam)
{
	D(d);
	switch (uMsg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0L);
		break;
	case WM_SIZE:
		{
			auto res = DefWindowProc(getWindowHandle(), uMsg, wParam, lParam);
			gm::GameMachine::instance().postMessage({ gm::GameMachineMessageType::WindowSizeChanged });
			return res;
		}
	default:
		break;
	}
	return ::DefWindowProc(getWindowHandle(), uMsg, wParam, lParam);
}

void GMUIGLWindow::dispose()
{
	D(d);
	gm::GMWindowHandle wnd = getWindowHandle();
	if (d->hRC)
	{
		if (!wglMakeCurrent(0, 0))
			gm_error(_L("release of DC and RC failed."));

		if (!wglDeleteContext(d->hRC))
			gm_error(_L("release Rendering Context failed."));

		d->hRC = 0;
	}

	if (d->hDC && !ReleaseDC(wnd, d->hDC))
	{
		gm_error(_L("release of Device Context failed."));
		d->hDC = 0;
	}

	if (wnd && !DestroyWindow(wnd))
	{
		gm_error(_L("could not release hWnd"));
		wnd = 0;
	}
}

#endif