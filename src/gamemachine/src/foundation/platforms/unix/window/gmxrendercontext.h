#ifndef __GMXRENDERCONTEXT_H__
#define __GMXRENDERCONTEXT_H__
#include <gmcommon.h>
#include <X11/Xlib.h>
#include <gmwindow.h>

BEGIN_NS

GM_PRIVATE_OBJECT(GMXRenderContext)
{
	GMint screen;
	GMint screenWidth, screenHeight, screenWidthMM, screenHeightMM;
	Window rootWindow;
};

class GMXRenderContext : public GMObject, public GMRenderContext
{
	GM_DECLARE_PRIVATE(GMXRenderContext)

public:
	GMXRenderContext(const char* displayName = nullptr);



	virtual void switchToContext() const override
	{
	}

	inline Display* getDisplay() const GM_NOEXCEPT
	{
		return const_cast<Display*>(s_display);
	}

	inline GMint getScreen() const GM_NOEXCEPT
	{
		D(d);
		return d->screen;
	}

	inline Window getRootWindow() const
	{
		D(d);
		return d->rootWindow;
	}

private:
	static Display* s_display;
};

END_NS
#endif