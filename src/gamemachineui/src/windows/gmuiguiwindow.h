#ifndef __GMUIGUIWINDOW_H__
#define __GMUIGUIWINDOW_H__
#include <gmuicommon.h>
#include "gmuidef.h"
#include <gminterfaces.h>
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
	virtual bool wndProc(gm::GMuint uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) override;

protected:
	virtual bool onCreate(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onClose(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onNcActivate(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onNcCalcSize(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onNcPaint(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onNcHitTest(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onSize(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onGetMinMaxInfo(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onSysCommand(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
	virtual bool onShowWindow(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* lRes) { return false; }
};
END_UI_NS

#endif