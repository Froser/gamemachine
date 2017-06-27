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
	for (auto& cmd : d->drawCmd)
	{
		drawCommand(cmd, hDC, rcPaint);
	}
}

void GMUIGraph::drawCommand(const GMGraphCommand& cmd, HDC hDC, const RECT& rcPaint)
{
	D(d);
	RECT& rc = const_cast<RECT&>(rcPaint);
	switch (cmd.type)
	{
	case GMGraphCommandType::Clear:
		{
			DuiLib::CRenderEngine::DrawGradient(hDC, rc, 0xFFFFFFFF, 0xFFFFFFFF, true, 8);
			break;
		}
	case GMGraphCommandType::Draw_Text:
		{
			RECT pos = { d->currentPos[0], d->currentPos[1], d->currentPos[0] + rc.right - rc.left, d->currentPos[1] + rc.bottom - rc.top };
			DuiLib::CRenderEngine::DrawText(hDC,
				GetManager(),
				pos,
				cmd.args.text.toStdWString().c_str(),
				RGB(cmd.args.color[0], cmd.args.color[1], cmd.args.color[2]),
				0,
				0);
			break;
		}
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