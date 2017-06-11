#ifndef __WINGL_WINDOW_H__
#define __WINGL_WINDOW_H__
#include "common.h"
#include "gmui/gmui.h"
BEGIN_NS

#if _WINDOWS

GM_PRIVATE_OBJECT(GMUIGLWindow)
{
	BYTE depthBits, stencilBits;
	HDC hDC;
	HGLRC hRC;
};

// Windows下的Window类
class GMUIGLWindow : public GMUIWindow
{
	typedef GMUIWindow Base;

	DECLARE_PRIVATE(GMUIGLWindow)

public:
	GMUIGLWindow();
	~GMUIGLWindow();

public:
	virtual GMUIWindowHandle create(const GMUIWindowAttributes& wndAttrs) override;
	virtual void swapBuffers() const override;

private:
	inline virtual GMUIStringPtr getWindowClassName() const override
	{
		return _L("gamemachine_mainWindow_class");
	}

	inline virtual GMuint getClassStyle() const override
	{
		return 0;
	}

private:
	void dispose();
};

#endif

END_NS
#endif