#ifndef __GMUICONTROLS_H__
#define __GMUICONTROLS_H__
#include "common.h"
#include <queue>
#include "foundation/vector.h"
#include "gmui.h"

#if _WINDOWS
#	include "uilib.h"
#endif

BEGIN_NS

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
	GMlong data[6];
	GMString text;
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
	virtual void drawText(const GMString& msg) = 0;
	virtual void drawRect(GMlong rgb, GMint width, GMint height) = 0;
	virtual void penEnter() = 0;
	virtual void penReturn(GMint yOffset) = 0;
	virtual void penForward(GMint xOffset, GMint yOffset) = 0;
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
	GMint currentPos[2] = { 0, 0 };
};

class GMUIGraph : public GMObject, public DuiLib::CControlUI, public IUIGraph
{
	DECLARE_PRIVATE(GMUIGraph)

	typedef DuiLib::CControlUI Base;

public:
	GMUIGraph(GMUIGUIWindow* parentWindow) { D(d); d->parentWindow = parentWindow; }

	// DuiLib::CControlUI
public:
	virtual bool DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl) override;

	// IUIGraph
public:
	virtual void beginDraw() override { D(d); d->drawCmd.clear(); }
	virtual void endDraw() override { D(d); d->parentWindow->refreshWindow(); };
	virtual void clearGraph() override;
	virtual void drawText(const GMString& msg) override;
	virtual void drawRect(GMlong rgb, GMint width, GMint height) override;
	virtual void penEnter() override;
	virtual void penReturn(GMint yOffset) override;
	virtual void penForward(GMint xOffset, GMint yOffset) override;

private:
	void addCommand(GMGraphCommand& cmd) { D(d); d->drawCmd.push_back(cmd); }
	void drawGraph(HDC hDC, const RECT& rcPaint);
	void drawCommand(const GMGraphCommand& cmd, HDC hDC, const RECT& rcPaint);
	void setPenPosition(GMint x, GMint y);
	void movePenPosition(GMint x, GMint y);
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
END_NS

#endif