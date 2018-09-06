#include "stdafx.h"
#include "gmxrendercontext.h"
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <gmwindow.h>

// Implement GMRenderContext
IWindow* GMRenderContext::getWindow() const
{
	return const_cast<IWindow*>(window);
}

IGraphicEngine* GMRenderContext::getEngine() const
{
	return const_cast<IGraphicEngine*>(engine);
}


Display* GMXRenderContext::s_display = nullptr;
GMint GMXRenderContext::s_instanceCount = 0;

GMXRenderContext::GMXRenderContext(IWindow* window, const char* displayName)
{
	D(d);
	if (!s_display)
	{
		s_display = XOpenDisplay(displayName);
		XSynchronize(s_display, True);
		if (!glXQueryExtension(s_display, NULL, NULL))
			gm_error(gm_dbg_wrap("OpenGL GLX extension not supported by s_display '{0}"), XDisplayName(displayName));
	}

	d->window = window;
	d->screen = DefaultScreen(s_display);
	d->rootWindow = RootWindow(s_display, d->screen);
	d->screenWidth = DisplayWidth(s_display, d->screen);
	d->screenHeight = DisplayHeight(s_display, d->screen);
	d->screenWidthMM = DisplayWidthMM(s_display, d->screen);
	d->screenHeightMM = DisplayHeightMM(s_display, d->screen);

	d->connection = ConnectionNumber(getDisplay());
	d->aDeleteWindow = getAtom("WM_DELETE_WINDOW");
	d->netWMSupported = isNetWMSupported();
	if (d->netWMSupported)
	{
		const Atom supported = getAtom("_NET_SUPPORTED");
		const Atom state = getAtom("_NET_WM_STATE");
		if (isHintPresent(getRootWindow(), supported, state))
		{
			const Atom fullScreen = getAtom("_NET_WM_STATE_FULLSCREEN");
			d->aState = state;

			if (isHintPresent(getRootWindow(), supported, fullScreen))
				d->aStateFullScreen = fullScreen;
		}

		d->aNetWMPid = getAtom("_NET_WM_PID");
		d->aClientMachine = getAtom("WM_CLIENT_MACHINE");
	}

	++s_instanceCount;
}

GMXRenderContext::~GMXRenderContext()
{
	D(d);
	--s_instanceCount;
	if (s_instanceCount == 0)
	{
		XSetCloseDownMode(getDisplay(), DestroyAll);
		XCloseDisplay(getDisplay());
	}
}

void GMXRenderContext::switchToContext() const
{
	D(d);
	glXMakeContextCurrent(getDisplay(), d->window->getWindowHandle(), d->window->getWindowHandle(), getGlxContext());
}

Atom GMXRenderContext::getAtom(const char* name)
{
	return XInternAtom(getDisplay(), name, False);
}

GMint GMXRenderContext::getWindowProperty(Window window, Atom property, Atom type, unsigned char** data)
{
	GMint status;
	Atom typeReturned = None;
	GMint tempFormat;
	GMulong numberOfElements;
	GMulong dtempBytesAfter;
	status = XGetWindowProperty(
		getDisplay(),
		window,
		property,
		0,
		LONG_MAX,
		False,
		type,
		&typeReturned,
		&tempFormat,
		&numberOfElements,
		&dtempBytesAfter,
		data
	);
	if (status != Success)
	{
		gm_error(gm_dbg_wrap("Retrieve window property failed."));
	}

	if (typeReturned != type)
	{
		gm_warning(gm_dbg_wrap("Type mismatch."));
		numberOfElements = 0;
	}
	return numberOfElements;
}

bool GMXRenderContext::isNetWMSupported()
{
	Atom wmCheck;
	Window** window1 = new Window*();
	GMint numberOfWindows = 0;
	bool netWMSupported = false;
	wmCheck = getAtom("_NET_SUPPORTING_WM_CHECK");
	numberOfWindows = getWindowProperty(getRootWindow(), wmCheck, XA_WINDOW, (unsigned char **)window1);

	if (numberOfWindows == 1)
	{
		Window** window2 = new Window*();
		numberOfWindows = getWindowProperty(**window1, wmCheck, XA_WINDOW, (unsigned char **) window2);

		if ((numberOfWindows == 1) && (**window1 == **window2))
			netWMSupported = true;

		XFree(*window2);
		delete window2;
	}

	XFree(*window1);
	delete window1;
	return netWMSupported;
}

bool GMXRenderContext::isHintPresent(Window window, Atom property, Atom hint)
{
	Atom* atoms = nullptr;
	bool supported = false;
	GMint numberOfAtoms = getWindowProperty(window, property, XA_ATOM, (unsigned char**) &atoms);

	for (GMint i = 0; i < numberOfAtoms; ++i)
	{
		if (atoms[i] == hint)
		{
			supported = true;
			break;
		}
	}

	XFree(atoms);
	return supported;
}