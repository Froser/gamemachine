#ifndef __GMUIGUIWINDOW_H__
#define __GMUIGUIWINDOW_H__
#include <gmuicommon.h>
#include "gmuidef.h"
#include <gamemachine.h>
#include "gmuiwindow.h"

BEGIN_UI_NS
// 一个标准的Windows GUI窗体
GM_PRIVATE_OBJECT(GMUIGUIWindow)
{
	GMUIPainter painter;
};

class GMUIGUIWindow : public GMUIWindow
{
	DECLARE_PRIVATE(GMUIGUIWindow);
	typedef GMUIWindow Base;

public:
	void hideWindow();
	bool isWindowVisible();
	void refreshWindow();

public:
	virtual LongResult handleMessage(gm::GMuint uMsg, UintPtr wParam, LongPtr lParam) override;

protected:
	virtual LongResult onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LongResult onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
};
END_UI_NS

#endif