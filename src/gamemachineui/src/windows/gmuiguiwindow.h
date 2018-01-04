#ifndef __GMUIGUIWINDOW_H__
#define __GMUIGUIWINDOW_H__
#include <gmuicommon.h>
#include "gmuidef.h"
#include <gamemachine.h>
#include "gmuiwindow.h"

BEGIN_UI_NS
// 一个标准的Windows GUI窗体
class GMUIGUIWindow : public GMUIWindow
{
	typedef GMUIWindow Base;

public:
	void hideWindow();
	bool isWindowVisible();
	void refreshWindow();

protected:
	virtual LRESULT wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, bool& bHandled) override;

protected:
	virtual LRESULT onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
	virtual LRESULT onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled) { bHandled = FALSE; return 0; }
};
END_UI_NS

#endif