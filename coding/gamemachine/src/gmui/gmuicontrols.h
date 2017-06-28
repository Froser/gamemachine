#ifndef __GMUICONTROLS_H__
#define __GMUICONTROLS_H__
#include "common.h"
#include <queue>

#if _WINDOWS
#	include "uilib.h"
#endif
#include "foundation/vector.h"

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
	virtual void addCommand(GMGraphCommand& cmd) = 0;
	virtual void clearCommands() = 0;
};

#if _WINDOWS

GM_PRIVATE_OBJECT(GMUIGraph)
{
	Vector<GMGraphCommand> drawCmd;

	GMint currentPos[2] = { 0, 0 };
};

class GMUIGraph : public GMObject, public DuiLib::CControlUI, public IUIGraph
{
	DECLARE_PRIVATE(GMUIGraph)

	typedef DuiLib::CControlUI Base;

public:
	GMUIGraph() = default;

	// DuiLib::CControlUI
public:
	virtual bool DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl) override;

	// IUIGraph
public:
	virtual void addCommand(GMGraphCommand& cmd) override { D(d); d->drawCmd.push_back(cmd); }
	virtual void clearCommands() override { D(d); d->drawCmd.clear(); }

private:
	void drawGraph(HDC hDC, const RECT& rcPaint);
	void drawCommand(const GMGraphCommand& cmd, HDC hDC, const RECT& rcPaint);
	void setPenPosition(GMint x, GMint y);
	void movePenPosition(GMint x, GMint y);
};

class GMUIDialogBuilder : public DuiLib::IDialogBuilderCallback
{
public:
	virtual DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
};

#endif //_WINDOWS
END_NS

#endif