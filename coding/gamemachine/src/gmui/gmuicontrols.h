#ifndef __GMUICONTROLS_H__
#define __GMUICONTROLS_H__
#include "common.h"
#include <queue>

#if _WINDOWS
#	include "uilib.h"
#endif
#include "foundation/vector.h"

BEGIN_NS

GM_INTERFACE(IUIGraph)
{
	virtual void clear() = 0;
	virtual void drawText(const GMString& str) = 0;
};

enum class GMGraphCommandType
{
	Clear,
	Draw_Text,
	Draw_Rect,
};

struct GMGraphCommandArgs
{
	GMbyte color[3];
	GMString text;
};

struct GMGraphCommand
{
	GMGraphCommandType type;
	GMGraphCommandArgs args;
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

	// IUIGraph
public:
	virtual void clear() override;
	virtual void drawText(const GMString& str) override;

	// DuiLib::CControlUI
public:
	virtual bool DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl) override;

public:
	void addCommand(GMGraphCommand& cmd) { D(d); d->drawCmd.push_back(cmd); }
	void clearCommands() { D(d); d->drawCmd.clear(); }

private:
	void drawGraph(HDC hDC, const RECT& rcPaint);
	void drawCommand(const GMGraphCommand& cmd, HDC hDC, const RECT& rcPaint);
};

class GMUIDialogBuilder : public DuiLib::IDialogBuilderCallback
{
public:
	virtual DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
};

#endif //_WINDOWS
END_NS

#endif