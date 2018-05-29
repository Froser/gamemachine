#include "stdafx.h"
#include "gmcontrols.h"
#include "gmcanvas.h"

void GMElementBlendColor::init(const GMVec4& defaultColor, const GMVec4& disabledColor, const GMVec4& hiddenColor)
{
	D(d);
	GM_FOREACH_ENUM_CLASS(state, GMControlState::Normal, GMControlState::EndOfControlState)
	{
		d->states[state] = defaultColor;
	};
	d->states[GMControlState::Disabled] = disabledColor;
	d->states[GMControlState::Hidden] = hiddenColor;
	d->current = hiddenColor;
}

void GMElementBlendColor::blend(GMControlState::State state, GMfloat elapsedTime, GMfloat rate)
{
	D(d);
	GMVec4 destColor = d->states[state];
	d->current = Lerp(d->current, destColor, 1/20.f);
}

void GMStyle::setTexture(GMuint texture, const GMRect& rc, const GMVec4& defaultTextureColor)
{
	D(d);
	d->texture = texture;
	d->rc = rc;
	d->textureColor.init(defaultTextureColor);
}

void GMStyle::setFont(GMuint font, const GMVec4& defaultColor)
{
	D(d);
	d->font = font;
	d->fontColor.init(defaultColor);
}

void GMStyle::setFontColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->fontColor.getStates()[state] = color;
}

void GMStyle::setTextureColor(GMControlState::State state, const GMVec4& color)
{
	D(d);
	d->textureColor.getStates()[state] = color;
}

void GMStyle::refresh()
{
	D(d);
	d->textureColor.setCurrent(d->textureColor.getStates()[GMControlState::Hidden]);
	d->fontColor.setCurrent(d->fontColor.getStates()[GMControlState::Hidden]);
}

GMControl::GMControl(GMCanvas* canvas)
{
	D(d);
	d->canvas = canvas;
}

GMControl::~GMControl()
{
	D(d);
	for (auto style : d->styles)
	{
		GM_delete(style);
	}
	GMClearSTLContainer(d->styles);
}

bool GMControl::handleKeyboard(GMSystemKeyEvent* event)
{
	bool handled = false;
	switch (event->getType())
	{
	case GMSystemEventType::KeyDown:
		handled = onKeyDown(event);
		break;
	case GMSystemEventType::KeyUp:
		handled = onKeyUp(event);
		break;
	}
	return handled;
}

bool GMControl::handleMouse(GMSystemMouseEvent* event)
{
	bool handled = false;
	switch (event->getType())
	{
	case GMSystemEventType::MouseDown:
		handled = onMousePress(event);
		break;
	case GMSystemEventType::MouseUp:
		handled = onMouseRelease(event);
		break;
	case GMSystemEventType::MouseMove:
		handled = onMouseMove(event);
		break;
	case GMSystemEventType::MouseDblClick:
		handled = onMouseDblClick(event);
		break;
	case GMSystemEventType::MouseWheel:
		handled = onMouseWheel(gm_cast<GMSystemMouseWheelEvent*>(event));
		break;
	}
	return handled;
}

void GMControl::updateRect()
{
	D(d);
	d->boundingBox.x = d->x;
	d->boundingBox.y = d->y;
	d->boundingBox.width = d->width;
	d->boundingBox.height = d->height;
}

void GMControl::refresh()
{
	D(d);
	d->mouseOver = false;
	d->hasFocus = false;

	for (auto style : d->styles)
	{
		style->refresh();
	}
}

bool GMControl::setStyle(GMuint index, GMStyle* style)
{
	D(d);
	bool hr = true;

	if (!style)
		return false;

	for (GMsize_t i = d->styles.size(); i <= index; i++)
	{
		GMStyle* newElement = new GMStyle();
		if (!newElement)
			return false;

		d->styles.push_back(newElement);
	}

	GMStyle* curElement = d->styles[index];
	*curElement = *style;
	return true;
}

GMControlStatic::GMControlStatic(GMCanvas* parent)
	: GMControl(parent)
{
	D_BASE(d, GMControl);
	d->type = GMControlType::Static;
}

void GMControlStatic::render(GMfloat elapsed)
{
	D(d);
	D_BASE(db, Base);
	if (!db->visible)
		return;

	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled())
		state = GMControlState::Disabled;

	GM_ASSERT(!db->styles.empty());
	GMStyle* style = db->styles[0];
	style->getFontColor().blend(state, elapsed);
	getParent()->drawText(getText(), style, db->boundingBox, false, false);
}

void GMControlStatic::setText(const GMString& text)
{
	D(d);
	d->text = text;
}

GMControlButton::GMControlButton(GMCanvas* parent)
	: GMControlStatic(parent)
{
	D_BASE(d, GMControl);
	d->type = GMControlType::Button;
}

bool GMControlButton::onMousePress(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseDblClick(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseRelease(GMSystemMouseEvent* event)
{
	return handleMouseRelease(event->getPoint());
}

bool GMControlButton::containsPoint(const GMPoint& pt)
{
	D_BASE(d, GMControl);
	return GM_inRect(d->boundingBox, pt);
}

bool GMControlButton::canHaveFocus()
{
	return getEnabled() && getVisible();
}

void GMControlButton::render(GMfloat elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, GMControl);
	GMint offsetX = 0, offsetY = 0;
	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled())
	{
		state = GMControlState::Disabled;
	}
	else if (d->pressed)
	{
		state = GMControlState::Pressed;
		offsetX = 1;
		offsetY = 2;
	}
	else if (getMouseOver())
	{
		state = GMControlState::MouseOver;
		offsetX = -1;
		offsetY = -2;
	}
	else if (hasFocus())
	{
		state = GMControlState::Focus;
	}

	GMCanvas* canvas = getParent();

	GMfloat blendRate = (state == GMControlState::Pressed) ? 0.0f : 0.8f;

	GMStyle* style = getStyle(0);
	style->getTextureColor().blend(state, elapsed, blendRate);
	style->getFontColor().blend(state, elapsed, blendRate);
	canvas->drawSprite(style, db->boundingBox, .8f);
	canvas->drawText(getText(), style, db->boundingBox, false, true);

	style = getStyle(1);
	style->getTextureColor().blend(state, elapsed, blendRate);
	style->getFontColor().blend(state, elapsed, blendRate);
	canvas->drawSprite(style, db->boundingBox, .8f);
	canvas->drawText(getText(), style, db->boundingBox, false, true);
}

bool GMControlButton::handleMousePressOrDblClick(const GMPoint& pt)
{
	if (!canHaveFocus())
		return false;

	D(d);
	D_BASE(db, GMControl);
	if (containsPoint(pt))
	{
		d->pressed = true;
		// TODO SetCapture

		if (!db->hasFocus)
			getParent()->requestFocus(this);
		return true;
	}

	return false;
}

bool GMControlButton::handleMouseRelease(const GMPoint& pt)
{
	if (!canHaveFocus())
		return false;

	D(d);
	D_BASE(db, GMControl);
	if (d->pressed)
	{
		d->pressed = false;
		// TODO ReleaseCapture

		GMCanvas* canvas = getParent();
		if (!canvas->canKeyboardInput())
			canvas->clearFocus();

		if (containsPoint(pt))
		{
			// TODO SendEvent
		}

		return true;
	}
	return false;
}