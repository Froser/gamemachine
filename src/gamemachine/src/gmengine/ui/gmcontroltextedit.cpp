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
	d->border = new GMControlTextEditBorder(widget);
	d->border->initStyles();
	d->buffer = new GMUniBuffer();
}

GMControlTextEdit::~GMControlTextEdit()
{
	D(d);
	GM_delete(d->border);
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

	GMint selectionStart = 0;
	GMint caretX = 0;
	placeCaret(caretX);

	d->border->render(elapsed);
}

void GMControlTextEdit::setSize(GMint width, GMint height)
{
	D(d);
	GMControl::setSize(width, height);
	d->border->setSize(width, height);
}

void GMControlTextEdit::setPosition(GMint x, GMint y)
{
	D(d);
	GMControl::setPosition(x, y);

	// border是以widget为参照，而不是以本控件为参照，所以要调整一次
	d->border->setPosition(x, y);
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
	d->border->initStyles();
}
