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
};

class GMUIGLWindow : public GMUIGameMachineWindowBase
{
	DECLARE_PRIVATE_AND_BASE(GMUIGLWindow, GMUIGameMachineWindowBase)

public:
	GMUIGLWindow();
	~GMUIGLWindow();

public:
	virtual gm::GMWindowHandle create(const gm::GMWindowAttributes& wndAttrs) override;
	virtual void update() override;

private:
	void swapBuffers() const;
	void dispose();
};

END_UI_NS
#endif