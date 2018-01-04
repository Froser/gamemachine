#ifndef __GMUIGLWINDOW_H__
#define __GMUIGLWINDOW_H__
#include "gmuidef.h"
#include "gmuiwindow.h"

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUIGLWindow)
{
	BYTE depthBits, stencilBits;
	HDC hDC;
	HGLRC hRC;
	gm::GMWindowHandle hWnd;
};

class GMUIGLWindow : public GMUIWindow
{
	DECLARE_PRIVATE_AND_BASE(GMUIGLWindow, GMUIWindow)

public:
	GMUIGLWindow();
	~GMUIGLWindow();

public:
	virtual gm::GMWindowHandle create(const gm::GMWindowAttributes& wndAttrs) override;
	virtual LongResult handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam) override;
	virtual void update() override;
	virtual gm::GMWindowHandle getWindowHandle() const override;

private:
	void swapBuffers() const;
	void dispose();
};

END_UI_NS
#endif