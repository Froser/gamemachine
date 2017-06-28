#include "stdafx.h"
#include "gmuicontrols.h"

#if _WINDOWS
#	define RGBA(r,g,b,a) RGB(r, g, b) | (a << 24)
const GMint MARGIN = 10;

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
	RECT rc = rcPaint;
	const RECT& p = this->GetClientPos();
	rc.left += p.left;
	rc.right += p.right;
	rc.top += p.top;
	rc.bottom += p.bottom;

	switch (cmd.type)
	{
	case GMGraphCommandType::Clear:
		{
			DuiLib::CRenderEngine::DrawGradient(hDC, rc, 0xFFFFFFFF, 0xFFFFFFFF, true, 8);
			RECT pos = this->GetClientPos();
			setPenPosition(MARGIN + pos.left, MARGIN + pos.top);
			break;
		}
	case GMGraphCommandType::Control_Return:
		{
			movePenPosition(0, cmd.args.data[0]);
			break;
		}
	case GMGraphCommandType::Control_Enter:
		{
			RECT pos = this->GetClientPos();
			setPenPosition(MARGIN + pos.left, d->currentPos[1]);
			break;
		}
	case GMGraphCommandType::Control_Forward:
		{
			movePenPosition(cmd.args.data[0], cmd.args.data[1]);
			break;
		}
	case GMGraphCommandType::Draw_Text:
		{
			RECT pos = {
				d->currentPos[0],
				d->currentPos[1],
				d->currentPos[0] + rc.right - rc.left,
				d->currentPos[1] + rc.bottom - rc.top
			};
			DuiLib::CRenderEngine::DrawText(hDC,
				GetManager(),
				pos,
				cmd.args.text.toStdWString().c_str(),
				RGB(cmd.args.data[0], cmd.args.data[1], cmd.args.data[2]),
				-1,
				DT_TOP | DT_LEFT);
			break;
		}
	case GMGraphCommandType::Draw_Rect:
		{
			RECT pos = {
				d->currentPos[0],
				d->currentPos[1],
				d->currentPos[0] + cmd.args.data[3], //w
				d->currentPos[1] + cmd.args.data[4]  //h
			};
			DuiLib::CRenderEngine::DrawGradient(
				hDC,
				pos,
				RGBA(cmd.args.data[0], cmd.args.data[1], cmd.args.data[2], 0xFF),
				RGBA(cmd.args.data[0], cmd.args.data[1], cmd.args.data[2], 0xFF),
				true,
				8
				);
			movePenPosition(cmd.args.data[3], 0);
			break;
		}
	default:
		break;
	}
}

void GMUIGraph::setPenPosition(GMint x, GMint y)
{
	D(d);
	d->currentPos[0] = x;
	d->currentPos[1] = y;
}

void GMUIGraph::movePenPosition(GMint x, GMint y)
{
	D(d);
	d->currentPos[0] += x;
	d->currentPos[1] += y;
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

#endif