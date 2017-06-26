#include "stdafx.h"
#include "gmuicontrols.h"

void GMUIGraph::clear()
{
	GMGraphCommand cmd = { GMGraphCommandType::Clear };
	addCommand(cmd);
}

void GMUIGraph::drawText(const GMString& str)
{

}

void GMUIGraph::drawGraph(HDC hDC, const RECT& rcPaint)
{
	D(d);
	while (!d->drawCmd.empty())
	{
		auto& cmd = d->drawCmd.front();
		drawCommand(cmd, hDC, rcPaint);
		d->drawCmd.pop();
	}
}

void GMUIGraph::drawCommand(const GMGraphCommand& cmd, HDC hDC, const RECT& rcPaint)
{
	switch (cmd.type)
	{
	case GMGraphCommandType::Clear:
		DuiLib::CRenderEngine::DrawRect(hDC, rcPaint, 1, RGB(1, 1, 1));
	default:
		break;
	}
}

bool GMUIGraph::DoPaint(HDC hDC, const RECT& rcPaint, DuiLib::CControlUI* pStopControl)
{
	drawGraph(hDC, rcPaint);
	return Base::DoPaint(hDC, rcPaint, pStopControl);
}

DuiLib::CControlUI* GMUIDialogBuilder::CreateControl(LPCTSTR pstrClass)
{
	if (wstrEqual(pstrClass, _L("GMGraph")))
	{
		return new GMUIGraph();
	}
	ASSERT(false);
	return nullptr;
}