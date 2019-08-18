#ifndef __GMXRENDERCONTEXT_H__
#define __GMXRENDERCONTEXT_H__
#include <gmcommon.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <GL/glx.h>
#include <gmwindow.h>

BEGIN_NS

struct GMXEventContext
{
	XEvent* event;
	IWindow* window;
};

GM_PRIVATE_CLASS(GMXRenderContext);
class GMXRenderContext : public GMObject, public GMRenderContext
{
	GM_DECLARE_PRIVATE(GMXRenderContext)
	GM_DECLARE_PROPERTY(GLXContext, GlxContext)
	GM_DECLARE_PROPERTY(GMint32, ScreenWidth)
	GM_DECLARE_PROPERTY(GMint32, ScreenHeight)
	GM_DECLARE_GETTER_ACCESSOR(bool, NetWMSupported, public)
	GM_DECLARE_GETTER_ACCESSOR(Atom, AtomDeleteWindow, public)
	GM_DECLARE_GETTER_ACCESSOR(Atom, AtomNetWMPid, public)
	GM_DECLARE_GETTER_ACCESSOR(Atom, AtomClientMachine, public)
	GM_DECLARE_GETTER_ACCESSOR(XIC, IC, public)

public:
	GMXRenderContext(IWindow* window, const char* displayName = nullptr);
	~GMXRenderContext();

public:
	virtual void switchToContext() const override;

public:
	Display* getDisplay() const GM_NOEXCEPT;
	GMint32 getScreen() const GM_NOEXCEPT;
	Window getRootWindow() const GM_NOEXCEPT;

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

END_NS
#endif