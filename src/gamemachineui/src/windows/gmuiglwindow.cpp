#include "stdafx.h"
#include "gmuiglwindow.h"
#include <gamemachine.h>
#include "gmuiinput.h"
#include <GL/glew.h>
#include <GL/wglew.h>

#define EXIT __exit
#define RUN_AND_CHECK(i) if (!(i)) { GM_ASSERT(false); goto EXIT; }

namespace
{
	const gm::GMwchar* g_classname = L"gamemachine_MainWindow_class";

	HWND createTempWindow()
	{
		LPWSTR lpszClassName = L"gamemachine_TempWindow_class";
		WNDCLASS wc;
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = ::DefWindowProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = NULL;
		wc.hIcon = ::LoadIcon(0, IDI_APPLICATION);
		wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);
		wc.lpszMenuName = NULL;
		wc.lpszClassName = lpszClassName;
		BOOL b = ::RegisterClass(&wc);
		GM_ASSERT(b);

		HWND wnd = CreateWindow(lpszClassName,
			L"GameMachineTempWindow",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			CW_USEDEFAULT,
			NULL,
			NULL,
			NULL,
			NULL);
		GM_ASSERT(wnd);
		::ShowWindow(wnd, SW_HIDE);
		::UpdateWindow(wnd);
		return wnd;
	}
}

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
	attrs.dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_BORDER | WS_CAPTION;

	// 在非全屏的时候才有效
	::AdjustWindowRectEx(&attrs.rc, attrs.dwStyle, FALSE, attrs.dwExStyle);

	if (!createWindow(attrs, g_classname))
	{
		DWORD s = GetLastError();
		gm_error(L"window created failed: %i", s);
		dispose();
		return false;
	}

	const gm::GMbyte colorDepth = 32, alphaBits = 8;
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

	HWND tmpWnd = NULL;
	HDC tmpDC = NULL;
	HGLRC tmpRC = NULL;
	gm::GMint pixelFormat;
	RUN_AND_CHECK(tmpWnd = createTempWindow());
	RUN_AND_CHECK(tmpDC = ::GetDC(tmpWnd));
	RUN_AND_CHECK(pixelFormat = ::ChoosePixelFormat(tmpDC, &pfd));
	RUN_AND_CHECK(::SetPixelFormat(tmpDC, pixelFormat, &pfd));
	RUN_AND_CHECK(tmpRC = wglCreateContext(tmpDC));
	RUN_AND_CHECK(wglMakeCurrent(tmpDC, tmpRC));
	RUN_AND_CHECK(GLEW_OK == glewInit());

	// 开始创建真正的Window
	gm::GMWindowHandle wnd = getWindowHandle();
	RUN_AND_CHECK(d->hDC = ::GetDC(wnd));
	
	gm::GMint pixAttribs[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,	GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,	GL_TRUE,
		WGL_DOUBLE_BUFFER_ARB,	GL_TRUE,
		WGL_SAMPLE_BUFFERS_EXT,	GL_TRUE,
		WGL_ACCELERATION_ARB,	WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,		24,
		WGL_ALPHA_BITS_ARB,		8,
		WGL_DEPTH_BITS_ARB,		24,
		WGL_STENCIL_BITS_ARB,	0,
		WGL_SAMPLES_ARB,		wndAttrs.samples,
		0
	};

	gm::GMint nFormat = 0;
	gm::GMuint nCount = 0;
	RUN_AND_CHECK(wglChoosePixelFormatARB(d->hDC, &pixAttribs[0], NULL, 1, &nFormat, (UINT*)&nCount));
	RUN_AND_CHECK(wglMakeCurrent(NULL, NULL));
	RUN_AND_CHECK(wglDeleteContext(tmpRC));
	RUN_AND_CHECK(::ReleaseDC(tmpWnd, tmpDC));
	RUN_AND_CHECK(::DestroyWindow(tmpWnd));
	RUN_AND_CHECK(::SetPixelFormat(d->hDC, nFormat, &pfd));
	RUN_AND_CHECK(d->hRC = wglCreateContext(d->hDC));
	RUN_AND_CHECK(wglMakeCurrent(d->hDC, d->hRC));
	return wnd;

EXIT:
	// 走到这里来说明流程失败
	dispose();
	if (tmpWnd && tmpDC)
		::ReleaseDC(tmpWnd, tmpDC);
	if (tmpRC)
		wglDeleteContext(tmpRC);
	if (tmpWnd)
		::DestroyWindow(tmpWnd);
	wglMakeCurrent(NULL, NULL);
	return NULL;
}

void GMUIGLWindow::swapBuffers() const
{
	D(d);
	::SwapBuffers(d->hDC);
}

bool GMUIGLWindow::wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes)
{
	D(d);
	D_BASE(db, GMUIWindow);

	bool handled = Base::wndProc(uMsg, wParam, lParam, lRes);
	if (handled)
		return handled;

	switch (uMsg)
	{
	case WM_DESTROY:
		::PostQuitMessage(0L);
		break;
	case WM_SIZE:
	{
		gm::GameMachine::instance().postMessage({ gm::GameMachineMessageType::WindowSizeChanged });
		break;
	}
	case WM_MOUSEWHEEL:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordWheel(true, GET_WHEEL_DELTA_WPARAM(wParam));
		break;
	}
	case WM_LBUTTONDOWN:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseDown(GMMouseButton_Left);
		break;
	}
	case WM_MOUSEMOVE:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseMove();
		break;
	}
	case WM_RBUTTONDOWN:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseDown(GMMouseButton_Right);
		break;
	}
	case WM_LBUTTONUP:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseUp(GMMouseButton_Left);
		break;
	}
	case WM_RBUTTONUP:
	{
		GMUIInput* input = gm_static_cast<GMUIInput*>(db->input);
		if (input)
			input->recordMouseUp(GMMouseButton_Right);
		break;
	}
	default:
		break;
	}

	*lRes = ::DefWindowProc(getWindowHandle(), uMsg, wParam, lParam);
	return true;
}

void GMUIGLWindow::update()
{
	swapBuffers();
	Base::update();
}

void GMUIGLWindow::dispose()
{
	D(d);
	gm::GMWindowHandle wnd = getWindowHandle();
	if (d->hRC)
	{
		if (!wglMakeCurrent(0, 0))
			gm_error(L"release of DC and RC failed.");

		if (!wglDeleteContext(d->hRC))
			gm_error(L"release Rendering Context failed.");

		d->hRC = 0;
	}

	if (d->hDC && !::ReleaseDC(wnd, d->hDC))
	{
		gm_error(L"release of Device Context failed.");
		d->hDC = 0;
	}

	if (wnd && !::DestroyWindow(wnd))
	{
		gm_error(L"could not release hWnd");
		wnd = 0;
	}
}

void GMUIGLWindow::showWindow()
{
	D(d);
	gm::GMWindowHandle hwnd = getWindowHandle();
	GM_ASSERT(::IsWindow(hwnd));
	if (!::IsWindow(hwnd)) return;
	::ShowWindow(hwnd, SW_SHOWNORMAL);
}