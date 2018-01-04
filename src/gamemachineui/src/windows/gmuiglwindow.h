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

class GMUIGLWindow : public GMUIWindow
{
	DECLARE_PRIVATE_AND_BASE(GMUIGLWindow, GMUIWindow)

public:
	GMUIGLWindow();
	~GMUIGLWindow();

protected:
	virtual LRESULT wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

public:
	virtual gm::GMWindowHandle create(const gm::GMWindowAttributes& wndAttrs) override;
	virtual void update() override;
	virtual bool handleMessage() override { return true; }
	virtual void showWindow() override;

private:
	bool createWindow(const gm::GMWindowAttributes& wndAttrs);
	void swapBuffers() const;
	void dispose();
};

END_UI_NS
#endif