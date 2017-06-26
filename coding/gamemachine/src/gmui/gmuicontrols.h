#ifndef __GMUICONTROLS_H__
#define __GMUICONTROLS_H__
#include "common.h"
#include <queue>

#if _WINDOWS
#	include "uilib.h"
#endif

BEGIN_NS

GM_INTERFACE(IUIGraph)
{
	virtual void clear() = 0;
	virtual void drawText(const GMString& str) = 0;
};

enum class GMGraphCommandType
{
	Clear,
	DrawText,
	DrawRect,
};

struct GMGraphCommandArgs
{
	GMRect pos;
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
	std::queue<GMGraphCommand> drawCmd;
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

private:
	void drawGraph(HDC hDC, const RECT& rcPaint);
	void drawCommand(const GMGraphCommand& cmd, HDC hDC, const RECT& rcPaint);
	void addCommand(GMGraphCommand& cmd) { D(d); d->drawCmd.push(cmd); }
};

class GMUIDialogBuilder : public DuiLib::IDialogBuilderCallback
{
public:
	virtual DuiLib::CControlUI* CreateControl(LPCTSTR pstrClass);
};

#endif //_WINDOWS
END_NS

#endif