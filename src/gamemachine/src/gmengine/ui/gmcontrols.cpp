#include "stdafx.h"
#include "gmcontrols.h"

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