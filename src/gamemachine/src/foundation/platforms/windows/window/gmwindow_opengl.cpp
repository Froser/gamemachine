#include "stdafx.h"
#include <GL/glew.h>
#include <GL/wglew.h>
#include "gmengine/ui/gmwindow.h"
#include "foundation/gamemachine.h"
#include "gmgl/gmglgraphic_engine.h"

#define EXIT __exit
#define RUN_AND_CHECK(i) if (!(i)) { GM_ASSERT(false); goto EXIT; }

namespace
{
	HWND createTempWindow()
	{
		LPWSTR lpszClassName = L"Gamemachine TempWindowClass";
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
		::RegisterClass(&wc);

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

GM_PRIVATE_OBJECT(GMWindow_OpenGL)
{
	BYTE depthBits, stencilBits;
	HDC hDC;
	HGLRC hRC;
};

class GMWindow_OpenGL : public GMWindow
{
	DECLARE_PRIVATE_AND_BASE(GMWindow_OpenGL, GMWindow)
	GM_FRIEND_CLASS(GMGLRenderContext)

public:
	GMWindow_OpenGL();
	~GMWindow_OpenGL();

public:
	virtual void msgProc(const GMMessage& message) override;
	virtual void onWindowCreated(const GMWindowAttributes& wndAttrs, GMWindowHandle handle) override;
	virtual IGraphicEngine* getGraphicEngine() override;
	virtual const IRenderContext* getContext() override;

private:
	void swapBuffers() const;
	void dispose();
};

class GMGLRenderContext : public GMRenderContext
{
public:
	GMGLRenderContext(GMWindow_OpenGL* window)
	{
		windowImpl = window;
	}

	virtual void switchToContext() const override
	{
		D_OF(d, windowImpl);
		BOOL b = wglMakeCurrent(d->hDC, d->hRC);
		GM_ASSERT(b);
	}

private:
	GMWindow_OpenGL* windowImpl;
};

GMWindow_OpenGL::GMWindow_OpenGL()
{
	D(d);
	d->depthBits = 24;
	d->stencilBits = 8;
}

GMWindow_OpenGL::~GMWindow_OpenGL()
{
	::SetWindowLongPtr(getWindowHandle(), GWLP_USERDATA, NULL);
	dispose();
}

void GMWindow_OpenGL::onWindowCreated(const GMWindowAttributes& wndAttrs, GMWindowHandle handle)
{
	D(d);
	GM_ASSERT(handle);

	gm::GMWindowAttributes attrs = wndAttrs;
	attrs.dwExStyle |= WS_EX_CLIENTEDGE;
	attrs.dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_SYSMENU | WS_BORDER | WS_CAPTION;

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
		WGL_COLOR_BITS_ARB,		colorDepth,
		WGL_ALPHA_BITS_ARB,		alphaBits,
		WGL_DEPTH_BITS_ARB,		d->depthBits,
		WGL_STENCIL_BITS_ARB,	d->stencilBits,
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
	return;

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
	return;
}

IGraphicEngine* GMWindow_OpenGL::getGraphicEngine()
{
	D_BASE(d, Base);
	if (!d->engine)
	{
		d->engine = new GMGLGraphicEngine(getContext());
	}
	return d->engine;
}

const IRenderContext* GMWindow_OpenGL::getContext()
{
	D_BASE(d, Base);
	if (!d->context)
	{
		GMGLRenderContext* context = new GMGLRenderContext(this);
		d->context = context;
		context->setWindow(this);
		context->setEngine(getGraphicEngine());
	}
	return d->context;
}

void GMWindow_OpenGL::swapBuffers() const
{
	D(d);
	::SwapBuffers(d->hDC);
}

void GMWindow_OpenGL::msgProc(const GMMessage& message)
{
	Base::msgProc(message);
	if (message.msgType == GameMachineMessageType::FrameUpdate)
		swapBuffers();
}

void GMWindow_OpenGL::dispose()
{
	D(d);
	gm::GMWindowHandle wnd = getWindowHandle();
	if (d->hRC)
	{
		if (!wglDeleteContext(d->hRC))
			gm_error(L"release Rendering Context failed.");

		d->hRC = 0;
	}

	if (d->hDC && !::ReleaseDC(wnd, d->hDC))
	{
		gm_error(L"release of Device Context failed.");
		d->hDC = 0;
	}
}

bool GMWindowFactory::createWindowWithOpenGL(GMInstance instance, OUT IWindow** window)
{
	(*window) = new GMWindow_OpenGL();
	if (*window)
		return true;
	return false;
}
