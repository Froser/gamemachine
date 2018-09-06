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
	GMint screen;
	GMint screenWidth, screenHeight, screenWidthMM, screenHeightMM;
	Window rootWindow;
	GLXContext glxContext;
	GMint connection = 0;
	Atom aDeleteWindow = None;
	Atom aState = None;
	Atom aStateFullScreen = None;
	Atom aNetWMPid = None;
	Atom aClientMachine = None;
	bool netWMSupported;
};

class GMXRenderContext : public GMObject, public GMRenderContext
{
	GM_DECLARE_PRIVATE(GMXRenderContext)
	GM_DECLARE_PROPERTY(GlxContext, glxContext, GLXContext)
	GM_DECLARE_GETTER(NetWMSupported, netWMSupported, bool)
	GM_DECLARE_GETTER(AtomDeleteWindow, aDeleteWindow, Atom)
	GM_DECLARE_GETTER(AtomNetWMPid, aNetWMPid, Atom)
	GM_DECLARE_GETTER(AtomClientMachine, aClientMachine, Atom)

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

	inline GMint getScreen() const GM_NOEXCEPT
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
	Atom getAtom(const char* name);
	GMint getWindowProperty(Window window, Atom property, Atom type, unsigned char** data);
	bool isNetWMSupported();
	bool isHintPresent(Window window, Atom property, Atom hint);

private:
	static Display* s_display;
	static GMint s_instanceCount;
};

END_NS
#endif