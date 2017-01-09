#ifndef __GMGLWINDOW_H__
#define __GMGLWINDOW_H__
#include "common.h"
#include "gmengine\controller\gamemachine.h"
BEGIN_NS

class GMGLWindow : public IWindow
{
public:
	GMGLWindow(char* cmdLine, char* windowTitle, bool fullScreen);

public:
	virtual void initWindowSize(GMfloat width, GMfloat height) override;
	virtual void setWindowResolution(GMfloat width, GMfloat height) override;
	virtual void setFullscreen(bool fullscreen) override;
	virtual void setWindowPosition(GMuint x, GMuint y) override;
	virtual void createWindow() override;
	virtual void startWindowLoop() override;
	virtual GMRect getWindowRect() override;

private:
	char m_windowTitle[128];
	bool m_fullscreen;
};

END_NS
#endif