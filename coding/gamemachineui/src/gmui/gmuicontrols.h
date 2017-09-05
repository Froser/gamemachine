#ifndef __GMUICONTROLS_H__
#define __GMUICONTROLS_H__
#include "common.h"
#include <gamemachine.h>
#include "gmui.h"

#if _WINDOWS
#	include "uilib.h"
#endif

BEGIN_UI_NS

enum class GMGraphCommandType
{
	Clear, // 清屏
	Control_Return, // 换行
	Control_Enter, // 回车
	Control_Forward, // 前行
	Draw_Text, // 绘制文本
	Draw_Rect, // 绘制矩形
};

struct GMGraphCommandArgs
{
	gm::GMlong data[6];
	gm::GMString text;
};

struct GMGraphCommand
{
	GMGraphCommandType type;
	GMGraphCommandArgs args;
};

GM_INTERFACE(IUIGraph)
{
	virtual void beginDraw() = 0;
	virtual void endDraw() = 0;
	virtual void clearGraph() = 0;
	virtual void drawText(const gm::GMString& msg) = 0;
	virtual void drawRect(gm::GMlong rgb, gm::GMint width, gm::GMint height) = 0;
	virtual void penEnter() = 0;
	virtual void penReturn(gm::GMint yOffset) = 0;
	virtual void penForward(gm::GMint xOffset, gm::GMint yOffset) = 0;
};

struct GMUIGraphGuard
{
	GMUIGraphGuard(IUIGraph* _g) : g(_g) { g->beginDraw(); }
	~GMUIGraphGuard() { g->endDraw(); }
private:
	IUIGraph* g;
};

#if _WINDOWS

GM_PRIVATE_OBJECT(GMUIGraph)
{
	GMUIGUIWindow* parentWindow = nullptr;
	Vector<GMGraphCommand> drawCmd;
	gm::GMint currentPos[2];
};

class GMUIGraph : public gm::GMObject, public DuiLib::CControlUI, public IUIGraph
{
	DECLARE_PRIVATE(GMUIGraph)

	typedef DuiLib::CControlUI Base;

public:
	GMUIGraph(GMUIGUIWindow* parentWindow) { D(d); d->parentWindow = parentWindow; d->currentPos[0] = d->currentPos[1] = 0; }

	// DuiLib::CControlUI
public:
	virtual bool DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl) override;

	// IUIGraph
public:
	virtual void beginDraw() override { D(d); d->drawCmd.clear(); }
	virtual void endDraw() override { D(d); d->parentWindow->refreshWindow(); };
	virtual void clearGraph() override;
	virtual void drawText(const gm::GMString& msg) override;
	virtual void drawRect(gm::GMlong rgb, gm::GMint width, gm::GMint height) override;
	virtual void penEnter() override;
	virtual void penReturn(gm::GMint yOffset) override;
	virtual void penForward(gm::GMint xOffset, gm::GMint yOffset) override;

private:
	void addCommand(GMGraphCommand& cmd) { D(d); d->drawCmd.push_back(cmd); }
	void drawGraph(HDC hDC, const RECT& rcPaint);
	void drawCommand(const GMGraphCommand& cmd, HDC hDC, const RECT& rcPaint);
	void setPenPosition(gm::GMint x, gm::GMint y);
	void movePenPosition(gm::GMint x, gm::GMint y);
};

GM_PRIVATE_OBJECT(GMUIDialogBuilder)
{
	GMUIGUIWindow* parentWindow;
};

class GMUIDialogBuilder : public DuiLib::IDialogBuilderCallback
{
	DECLARE_PRIVATE(GMUIDialogBuilder)

public:
	GMUIDialogBuilder(GMUIGUIWindow* parentWindow) { D(d); d->parentWindow = parentWindow; }

public:
	virtual DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
};

#endif //_WINDOWS
END_UI_NS

#endif