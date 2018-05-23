#include "stdafx.h"
#include "gmcontrols.h"
#include "gmcanvas.h"

void GMElementBlendColor::init(const GMVec4& defaultColor, const GMVec4& disabledColor, const GMVec4& hiddenColor)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(state, GMControlState::Normal, GMControlState::EndOfControlState)
	{
		d->states[(GMuint)state] = defaultColor;
	};
	d->states[(GMuint)GMControlState::Disabled] = disabledColor;
	d->states[(GMuint)GMControlState::Hidden] = hiddenColor;
	d->current = hiddenColor;
}

void GMElementBlendColor::blend(GMControlState state, GMfloat elapsedTime, GMfloat rate)
{
	D(d);
	GMVec4 destColor = d->states[(GMuint)state];
	d->current = Lerp(d->current, destColor, Pow(rate, 30 * elapsedTime));
}

GMControl::GMControl(GMCanvas* canvas)
{
	D(d);
	d->canvas = canvas;
}

GMControl::~GMControl()
{
	D(d);
	for (auto element : d->elements)
	{
		GM_delete(element);
	}
	GMClearSTLContainer(d->elements);
}

void GMControl::updateRect()
{
	D(d);
	d->rcBoundingBox.x = d->x;
	d->rcBoundingBox.y = d->y;
	d->rcBoundingBox.width = d->width;
	d->rcBoundingBox.height = d->height;
}

void GMControl::refresh()
{
	D(d);
	d->mouseOver = false;
	d->hasFocus = false;

	for (auto element : d->elements)
	{
		element->refresh();
	}
}

bool GMControl::setElement(GMuint index, GMElement* element)
{
	D(d);
	bool hr = true;

	if (!element)
		return false;

	for (size_t i = d->elements.size(); i <= index; i++)
	{
		GMElement* newElement = new GMElement();
		if (!newElement)
			return false;

		d->elements.push_back(newElement);
	}

	GMElement* curElement = d->elements[index];
	*curElement = *element;
	return true;
}

GMControlStatic::GMControlStatic(GMCanvas* parent)
	: GMControl(parent)
{
	D_BASE(d, Base);
	d->type = GMControlType::Static;
}

void GMControlStatic::render(GMfloat elapsed)
{
	D(d);
	D_BASE(db, Base);
	if (!db->visible)
		return;

	GMControlState state = GMControlState::Normal;
	if (db->enabled == false)
		state = GMControlState::Disabled;

	GM_ASSERT(!db->elements.empty());
	GMElement* element = db->elements[0];
	element->getTextureColor().blend(state, elapsed);
	db->canvas->drawText(d->text, element, db->rcBoundingBox, false, -1, false);
}

void GMControlStatic::setText(const GMString& text)
{
	D(d);
	d->text = text;
}