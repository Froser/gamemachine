#include "stdafx.h"
#include "gmuicontrols.h"

#if _WINDOWS
#	define RGBA(r,g,b,a) RGB(r, g, b) | (a << 24)
const gm::GMint MARGIN = 10;

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
			d->currentPos[0] = d->currentPos[1] = 0;
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

void GMUIGraph::setPenPosition(gm::GMint x, gm::GMint y)
{
	D(d);
	d->currentPos[0] = x;
	d->currentPos[1] = y;
}

void GMUIGraph::movePenPosition(gm::GMint x, gm::GMint y)
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


void GMUIGraph::clearGraph()
{
	GMGraphCommand cmd = { GMGraphCommandType::Clear };
	addCommand(cmd);
}

void GMUIGraph::drawText(const gm::GMString& msg)
{
	GMGraphCommand cmd = { GMGraphCommandType::Draw_Text,{ 0, 0, 0, 0, 0, 0, msg } };
	addCommand(cmd);
}

void GMUIGraph::drawRect(gm::GMlong rgb, gm::GMint width, gm::GMint height)
{
	gm::GMbyte r = GetRValue(rgb), g = GetGValue(rgb), b = GetBValue(rgb);
	GMGraphCommand cmd = { GMGraphCommandType::Draw_Rect,{ r, g, b, width, height } };
	addCommand(cmd);
}

void GMUIGraph::penEnter()
{
	GMGraphCommand cmd = { GMGraphCommandType::Control_Enter };
	addCommand(cmd);
}

void GMUIGraph::penReturn(gm::GMint yOffset)
{
	GMGraphCommand cmd = { GMGraphCommandType::Control_Return,{ yOffset } };
	addCommand(cmd);
}

void GMUIGraph::penForward(gm::GMint xOffset, gm::GMint yOffset)
{
	GMGraphCommand cmd = { GMGraphCommandType::Control_Forward,{ xOffset, yOffset } };
	addCommand(cmd);
}

DuiLib::CControlUI* GMUIDialogBuilder::CreateControl(LPCTSTR pstrClass)
{
	D(d);
	if (wstrEqual(pstrClass, _L("GMGraph")))
	{
		return new GMUIGraph(d->parentWindow);
	}
	ASSERT(false);
	return nullptr;
}
#endif