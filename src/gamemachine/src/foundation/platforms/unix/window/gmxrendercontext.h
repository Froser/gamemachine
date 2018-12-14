#ifndef __GMXRENDERCONTEXT_H__
#define __GMXRENDERCONTEXT_H__
#include <gmcommon.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <gmwindow.h>

BEGIN_NS

GM_PRIVATE_OBJECT(GMXRenderContext)
{
	IWindow* window = nullptr;
	GMint32 screen;
	GMint32 screenWidth, screenHeight, screenWidthMM, screenHeightMM;
	Window rootWindow;
	GLXContext glxContext;
	GMint32 connection = 0;
	Atom aDeleteWindow = None;
	Atom aState = None;
	Atom aStateFullScreen = None;
	Atom aNetWMPid = None;
	Atom aClientMachine = None;
	bool netWMSupported;
	XIM im;
	XIC ic;
};

class GMXRenderContext : public GMObject, public GMRenderContext
{
	GM_DECLARE_PRIVATE(GMXRenderContext)
	GM_DECLARE_PROPERTY(GlxContext, glxContext)
	GM_DECLARE_PROPERTY(ScreenWidth, screenWidth)
	GM_DECLARE_PROPERTY(ScreenHeight, screenHeight)
	GM_DECLARE_GETTER(NetWMSupported, netWMSupported)
	GM_DECLARE_GETTER(AtomDeleteWindow, aDeleteWindow)
	GM_DECLARE_GETTER(AtomNetWMPid, aNetWMPid)
	GM_DECLARE_GETTER(AtomClientMachine, aClientMachine)
	GM_DECLARE_GETTER(IC, ic)

public:
	GMXRenderContext(IWindow* window, const char* displayName = nullptr);
	~GMXRenderContext();

public:
	virtual void switchToContext() const override;

public:
	inline Display* getDisplay() const GM_NOEXCEPT
	{
		return const_cast<Display*>(s_display);
	}

	inline GMint32 getScreen() const GM_NOEXCEPT
	{
		D(d);
		return d->screen;
	}

	inline Window getRootWindow() const GM_NOEXCEPT
	{
		D(d);
		return d->rootWindow;
	}

private:
	void initX(IWindow* window, const char* displayName);
	void initIM();
	Atom getAtom(const char* name);
	GMint32 getWindowProperty(Window window, Atom property, Atom type, unsigned char** data);
	bool isNetWMSupported();
	bool isHintPresent(Window window, Atom property, Atom hint);

private:
	static Display* s_display;
	static GMint32 s_instanceCount;
};

struct GMXEventContext
{
	XEvent* event;
	IWindow* window;
};

END_NS
#endif