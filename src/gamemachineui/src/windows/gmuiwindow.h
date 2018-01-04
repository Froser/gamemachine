#ifndef __GMUIWINDOW_H__
#define __GMUIWINDOW_H__
#include <gmuicommon.h>
#include "gmuidef.h"
#include <gamemachine.h>

BEGIN_UI_NS

GM_PRIVATE_OBJECT(GMUIWindow)
{
	gm::IInput* input = nullptr;
};

class GMUIWindow : public gm::GMObject, public gm::IWindow
{
	DECLARE_PRIVATE(GMUIWindow)

public:
	~GMUIWindow();

public:
	operator gm::GMWindowHandle() const { return getWindowHandle(); }

	// IWindow
public:
	virtual gm::GMWindowHandle create(const gm::GMWindowAttributes& attrs) = 0;
	virtual gm::GMWindowHandle getWindowHandle() const = 0;
	virtual bool handleMessage() = 0;
	virtual void showWindow() = 0;

	virtual gm::IInput* getInputMananger() override;
	virtual void update() override;
	virtual gm::GMRect getWindowRect() override;
	virtual gm::GMRect getClientRect() override;
	virtual void centerWindow() override;
	virtual bool isWindowActivate() override;
	virtual void setLockWindow(bool lock) override;
	virtual bool event(const gm::GameMachineMessage& msg) override { return false; }

	// 新虚方法
protected:
	virtual LongResult handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam) = 0;
};

END_UI_NS

#endif