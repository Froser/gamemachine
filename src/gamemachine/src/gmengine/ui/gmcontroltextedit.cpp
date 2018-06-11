#include "stdafx.h"
#include <gmunibuffer.h>
#include "gmcontroltextedit.h"

class GMControlTextEditBorder : public GMControlBorder
{
public:
	using GMControlBorder::GMControlBorder;

protected:
	virtual void initStyles();
};

GMControlTextEdit::GMControlTextEdit(GMWidget* widget)
	: GMControl(widget)
{
	D(d);
	d->borderControl = new GMControlTextEditBorder(widget);
	d->borderControl->initStyles();
	d->buffer = new GMUniBuffer();
}

GMControlTextEdit::~GMControlTextEdit()
{
	D(d);
	GM_delete(d->borderControl);
	GM_delete(d->buffer);
}

void GMControlTextEditBorder::initStyles()
{
	D(d);
	GMWidget* widget = getParent();
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

	GMint selectionStartX = 0;
	GMint caretX = 0;
	placeCaret(caretX);

	d->borderControl->render(elapsed);

	// 计算首个能显示的字符
	GMint firstX;
	d->buffer->CPtoX(d->firstVisibleCP, false, &firstX);

	// 计算选区
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
		rcSelection.width = selectionRightX - selectionRightX;
		rcSelection.height = d->rcText.height;
		GMRect rc = GM_intersectRect(rcSelection, d->rcText);
		widget->drawRect(d->selectionBackColor, rc, .99f);
	}

	//TODO
	d->textStyle.getFontColor().setCurrent(d->textColor);
	widget->drawText(d->buffer->getBuffer(), d->textStyle, d->rcText);
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

void GMControlTextEdit::initStyles()
{
	D(d);
	GMStyle textStyle;
	textStyle.setFont(0, GMVec4(0, 0, 0, 1));
	d->borderControl->initStyles();
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
