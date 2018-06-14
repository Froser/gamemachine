#include "stdafx.h"
#include <gmunibuffer.h>
#include "gmcontroltextedit.h"
#include "foundation/gamemachine.h"

GM_DEFINE_SIGNAL(GMControlTextEdit::textChanged);

class GMControlTextEditBorder : public GMControlBorder
{
public:
	GMControlTextEditBorder(GMWidget* widget) : GMControlBorder(widget) { initStyles(widget); }

private:
	void initStyles(GMWidget* widget);
};

GMControlTextEdit::GMControlTextEdit(GMWidget* widget)
	: GMControl(widget)
{
	D(d);
	d->borderControl = new GMControlTextEditBorder(widget);
	d->buffer = new GMUniBuffer();
	initStyles(widget);
}

GMControlTextEdit::~GMControlTextEdit()
{
	D(d);
	GM_delete(d->borderControl);
	GM_delete(d->buffer);
}

void GMControlTextEditBorder::initStyles(GMWidget* widget)
{
	D(d);
	d->borderStyle.setTexture(GMWidgetResourceManager::Skin, widget->getArea(GMTextureArea::TextEditBorderArea));
	d->borderStyle.setTextureColor(GMControlState::Normal, GMVec4(1.f, 1.f, 1.f, 1.f));
}

void GMControlTextEdit::render(GMfloat elapsed)
{
	D(d);
	if (!getVisible())
		return;

	if (!d->buffer->getContext())
		d->buffer->setContext(getParent()->getParentWindow()->getContext());

	placeCaret(d->cp);
	d->borderControl->render(elapsed);

	// 计算首个能显示的字符
	GMint firstX;
	d->buffer->CPtoX(d->firstVisibleCP, false, &firstX);

	// 计算选区
	GMint caretX;
	GMint selectionStartX = 0;
	d->buffer->CPtoX(d->cp, false, &caretX);
	if (d->cp != d->selectionStartCP)
		d->buffer->CPtoX(d->selectionStartCP, false, &selectionStartX);
	else
		selectionStartX = caretX;

	GMWidget* widget = getParent();
	GMRect rcSelection;
	if (d->cp != d->selectionStartCP)
	{
		// 如果当前位置不等于选定开始的位置，则确定一个选区
		GMint selectionLeftX = caretX, selectionRightX = selectionStartX;
		if (selectionLeftX > selectionRightX)
		{
			GM_SWAP(selectionLeftX, selectionRightX);
		}

		rcSelection.x = selectionLeftX + (d->rcText.x - firstX);
		rcSelection.y = d->rcText.y;
		rcSelection.width = selectionRightX - selectionLeftX;
		rcSelection.height = d->rcText.height;
		GMRect rc = GM_intersectRect(rcSelection, d->rcText);
		widget->drawRect(d->selectionBackColor, rc, .99f);
	}

	//TODO
	d->textStyle.getFontColor().setCurrent(d->textColor);
	widget->drawText(d->buffer->getBuffer(), d->textStyle, d->rcText);

	// 闪烁光标
	GMfloat time = GM.getGameMachineRunningStates().elapsedTime;
	if (time - d->lastBlink >= d->deltaBlink)
	{
		d->caretOn = !d->caretOn;
		d->lastBlink = time;
	}

	if (hasFocus() && d->caretOn && d->showCaret)
		renderCaret(firstX, caretX);
}

void GMControlTextEdit::setSize(GMint width, GMint height)
{
	D(d);
	GMControl::setSize(width, height);
	d->borderControl->setSize(width, height);
}

void GMControlTextEdit::setPosition(GMint x, GMint y)
{
	D(d);
	GMControl::setPosition(x, y);

	// border是以widget为参照，而不是以本控件为参照，所以要调整一次
	d->borderControl->setPosition(x, y);
}

bool GMControlTextEdit::onKeyDown(GMSystemKeyEvent* event)
{
	if (!getVisible() || !getEnabled())
		return false;

	bool handled = false;

	GMKey key = event->getKey();
	switch (key)
	{
	case GMKey_Tab:
		break;
	case GMKey_Left:
	case GMKey_Right:
	{
		moveCaret(key == GMKey_Right, !!(event->getModifier() & GMModifier_Ctrl), !!(event->getModifier() & GMModifier_Shift));
		handled = true;
		break;
	}
	case GMKey_Up:
	case GMKey_Down:
	{
		// 上下不会切换焦点
		handled = true;
		break;
	}
	}

	return handled;
}

bool GMControlTextEdit::onChar(GMSystemCharEvent* event)
{
	D(d);
	switch (event->getKey())
	{
	case GMKey_Back:
	{
		if (d->selectionStartCP != d->cp)
		{
			deleteSelectionText();
		}
		else if (d->cp > 0)
		{
			placeCaret(d->cp - 1);
			d->selectionStartCP = d->cp;
			d->buffer->removeChar(d->cp);
			emit(textChanged);
			resetCaretBlink();
		}
		return true;
	}
	}

	if (d->selectionStartCP != d->cp)
		deleteSelectionText();

	if (!d->insertMode && d->cp < d->buffer->getLength())
	{
		d->buffer->setChar(d->cp, event->getCharacter());
		placeCaret(d->cp + 1);
		d->selectionStartCP = d->cp;
	}
	else
	{
		if (d->buffer->insertChar(d->cp, event->getCharacter()))
		{
			placeCaret(d->cp + 1);
			d->selectionStartCP = d->cp;
		}
	}
	resetCaretBlink();
	emit(textChanged);

	return true;
}

bool GMControlTextEdit::canHaveFocus()
{
	D(d);
	return getVisible() && getEnabled();
}

void GMControlTextEdit::setText(const GMString& text)
{
	D(d);
	d->buffer->setBuffer(text);
}

void GMControlTextEdit::placeCaret(GMint cp)
{
	D(d);
	GM_ASSERT(cp >= 0 && cp <= d->buffer->getLength());
	d->cp = cp;

	GMint firstX, x, x2;
	d->buffer->CPtoX(d->firstVisibleCP, false, &firstX);
	d->buffer->CPtoX(cp, false, &x); //lead
	if (cp == d->buffer->getLength())
		x2 = x;
	else
		d->buffer->CPtoX(cp, true, &x2); //trail

	if (x < firstX)
	{
		// 此时字符所在的位置比可见位置小，我们需要调整滚动条到其可见
		d->firstVisibleCP = x;
	}
	else if (x2 > firstX + d->rcText.width)
	{
		GMint xNewLeft = x2 - d->rcText.width;
		GMint cpNewFirst;
		bool newTrail;
		d->buffer->XtoCP(xNewLeft, &cpNewFirst, &newTrail);
		GMint xNewFirst;
		d->buffer->CPtoX(cpNewFirst, false, &xNewFirst);
		if (xNewFirst < xNewLeft)
			++cpNewFirst;

		d->firstVisibleCP = cpNewFirst;
	}
}

void GMControlTextEdit::moveCaret(bool next, bool newItem, bool select)
{
	D(d);
	if (newItem && next)
	{
		d->buffer->getNextItemPos(d->cp, &d->cp);
		placeCaret(d->cp);
	}
	else if (newItem && !next)
	{
		d->buffer->getPriorItemPos(d->cp, &d->cp);
		placeCaret(d->cp);
	}
	else if (next && d->cp < d->buffer->getLength())
	{
		placeCaret(d->cp + 1);
	}
	else if (!next && d->cp > 0)
	{
		placeCaret(d->cp - 1);
	}
	if (!select)
	{
		d->selectionStartCP = d->cp;
	}
	resetCaretBlink();
}

void GMControlTextEdit::deleteSelectionText()
{
	D(d);
	GMint firstCp = Min(d->cp, d->selectionStartCP);
	GMint lastCp = Max(d->cp, d->selectionStartCP);
	placeCaret(firstCp);
	d->selectionStartCP = d->cp;
	bool s = d->buffer->removeChars(firstCp, lastCp);
	GM_ASSERT(s);
}

void GMControlTextEdit::resetCaretBlink()
{
	D(d);
	d->lastBlink = GM.getGameMachineRunningStates().elapsedTime;
	d->caretOn = true;
}

void GMControlTextEdit::initStyles(GMWidget* widget)
{
	D(d);
	GMStyle textStyle;
	textStyle.setFont(0);
	textStyle.setFontColor(GMVec4(0, 0, 0, 1));
	d->textStyle = textStyle;
}

void GMControlTextEdit::updateRect()
{
	GMControl::updateRect();
	
	D(d);
	d->rcText = boundingRect();
	d->rcText.x += d->borderWidth;
	d->rcText.width -= d->borderWidth * 2;
	d->rcText.y += d->borderWidth;
	d->rcText.height -= d->borderWidth * 2;
}

void GMControlTextEdit::renderCaret(GMint firstX, GMint caretX)
{
	D(d);
	GMRect rc = {
		d->rcText.x - firstX + caretX - 1,
		d->rcText.y,
		2,
		d->rcText.height
	};
	if (!d->insertMode)
	{
		// 改写模式
		GMint rightEdgeX;
		d->buffer->CPtoX(d->cp, true, &rightEdgeX);
		rc.width = rightEdgeX - firstX;
	}

	GMWidget* widget = getParent();
	widget->drawRect(d->caretColor, rc, .99f);
}