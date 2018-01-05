﻿#ifndef __GMUIGLWINDOW_H__
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

class GMUIGLWindow : public GMUIWindow
{
	DECLARE_PRIVATE_AND_BASE(GMUIGLWindow, GMUIWindow)

public:
	GMUIGLWindow();
	~GMUIGLWindow();

protected:
	virtual bool wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) override;

public:
	virtual gm::GMWindowHandle create(const gm::GMWindowAttributes& wndAttrs) override;
	virtual void update() override;
	virtual void showWindow() override;

private:
	void swapBuffers() const;
	void dispose();
};

END_UI_NS
#endif