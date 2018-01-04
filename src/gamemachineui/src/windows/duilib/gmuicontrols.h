#ifndef __GMUICONTROLS_H__
#define __GMUICONTROLS_H__
#include <gamemachine.h>
#include "../gmuidef.h"
#include "gmuiduilibwindow.h"

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

GM_PRIVATE_OBJECT(GMUIGraph)
{
	GMUIDuiLibWindow* parentWindow = nullptr;
	Vector<GMGraphCommand> drawCmd;
	gm::GMint currentPos[2];
};

class GMUIGraph : public gm::GMObject, public DuiLib::CControlUI, public IUIGraph
{
	DECLARE_PRIVATE_AND_BASE(GMUIGraph, DuiLib::CControlUI)

public:
	GMUIGraph(GMUIDuiLibWindow* parentWindow) { D(d); d->parentWindow = parentWindow; d->currentPos[0] = d->currentPos[1] = 0; }

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

class GMUIDuiLibWindow;
GM_PRIVATE_OBJECT(GMUIDialogBuilder)
{
	GMUIDuiLibWindow* parentWindow;
};

class GMUIDialogBuilder : public DuiLib::IDialogBuilderCallback
{
	DECLARE_PRIVATE_AND_BASE(GMUIDialogBuilder, DuiLib::IDialogBuilderCallback)

public:
	GMUIDialogBuilder(GMUIDuiLibWindow* parentWindow) { D(d); d->parentWindow = parentWindow; }

public:
	virtual DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
};

END_UI_NS

#endif