#include "stdafx.h"
#include "gmcontrols.h"
#include "gmwidget.h"

GMControl::GMControl(GMWidget* widget)
{
	D(d);
	d->widget = widget;
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
		handled = onMouseDown(event);
		break;
	case GMSystemEventType::MouseUp:
		handled = onMouseUp(event);
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
}

GMStyle& GMControl::getStyle(StyleType style)
{
	static GMStyle s_style;
	return s_style;
}

void GMControlStatic::render(GMfloat elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);

	GMControlState::State state = GMControlState::Normal;
	if (!getEnabled())
		state = GMControlState::Disabled;

	GMStyle& foreStyle = getStyle((GMControl::StyleType)ForeStyle);
	foreStyle.getFontColor().blend(state, elapsed);
	getParent()->drawText(getText(), foreStyle, db->boundingBox, foreStyle.getShadowStyle().hasShadow, false);
}

void GMControlStatic::refresh()
{
	D(d);
	Base::refresh();
	d->foreStyle.refresh();
}

GMStyle& GMControlStatic::getStyle(Base::StyleType style)
{
	D(d);
	Base::StyleType s = (Base::StyleType)style;
	switch (s)
	{
	case GMControlStatic::ForeStyle:
		return d->foreStyle;
	default:
		return Base::getStyle(style);
	}
}

void GMControlStatic::initStyles()
{
	D(d);
	d->foreStyle.setFont(0);
	d->foreStyle.setFontColor(GMControlState::Disabled, GMVec4(.87f, .87f, .87f, .87f));
}

void GMControlStatic::setText(const GMString& text)
{
	D(d);
	d->text = text;
}

GM_DEFINE_SIGNAL(GMControlButton::click);

void GMControlButton::refresh()
{
	D(d);
	Base::refresh();
	d->fillStyle.refresh();
}

GMStyle& GMControlButton::getStyle(GMControl::StyleType style)
{
	D(d);
	D_BASE(db, Base);
	GMControl::StyleType s = (GMControl::StyleType)style;
	switch (s)
	{
	case GMControlButton::ForeStyle:
		return db->foreStyle;
	case GMControlButton::FillStyle:
		return d->fillStyle;
	default:
		return Base::getStyle(style);
	}
}

bool GMControlButton::onMouseDown(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseDblClick(GMSystemMouseEvent* event)
{
	return handleMousePressOrDblClick(event->getPoint());
}

bool GMControlButton::onMouseUp(GMSystemMouseEvent* event)
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

bool GMControlButton::onKeyDown(GMSystemKeyEvent* event)
{
	D(d);
	if (event->getKey() == GMKey_Space)
	{
		d->pressed = true;
		return true;
	}
	return false;
}

bool GMControlButton::onKeyUp(GMSystemKeyEvent* event)
{
	D(d);
	if (event->getKey() == GMKey_Space)
	{
		if (d->pressed)
		{
			d->pressed = false;
			emit(click);
			return true;
		}
	}
	return false;
}

void GMControlButton::render(GMfloat elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);
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

	GMRect rc = boundingRect();
	rc.x += offsetX;
	rc.width -= offsetX * 2;
	rc.y += offsetY;
	rc.height -= offsetY * 2;

	GMWidget* widget = getParent();
	GMfloat blendRate = (state == GMControlState::Pressed) ? 0.0f : 0.8f;

	GMStyle& foreStyle = getStyle((GMControl::StyleType)ForeStyle);
	foreStyle.getTextureColor().blend(state, elapsed, blendRate);
	foreStyle.getFontColor().blend(state, elapsed, blendRate);
	widget->drawSprite(foreStyle, rc, .8f);
	widget->drawText(getText(), foreStyle, rc, foreStyle.getShadowStyle().hasShadow, true);

	GMStyle& fillStyle = getStyle((GMControl::StyleType)FillStyle);
	fillStyle.getTextureColor().blend(state, elapsed, blendRate);
	fillStyle.getFontColor().blend(state, elapsed, blendRate);
	widget->drawSprite(fillStyle, rc, .8f);
	widget->drawText(getText(), fillStyle, rc, fillStyle.getShadowStyle().hasShadow, true);
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
		getParent()->getParentWindow()->setWindowCapture(true);

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
		getParent()->getParentWindow()->setWindowCapture(false);

		GMWidget* widget = getParent();
		if (!widget->canKeyboardInput())
			widget->clearFocus();

		if (containsPoint(pt))
		{
			emit(click);
		}

		return true;
	}
	return false;
}

void GMControlButton::initStyles()
{
	D(d);
	D_BASE(db, Base);

	GMWidget* widget = getParent();
	db->foreStyle.setTexture(GMWidgetResourceManager::Skin, widget->getArea(GMTextureArea::ButtonArea));
	db->foreStyle.setFont(0);
	db->foreStyle.setTextureColor(GMControlState::Normal, GMVec4(1.f, 1.f, 1.f, .58f));
	db->foreStyle.setTextureColor(GMControlState::Pressed, GMVec4(0, 0, 0, .78f));
	db->foreStyle.setFontColor(GMControlState::Normal, GMVec4(.3f, .3f, .3f, 1.f));
	db->foreStyle.setFontColor(GMControlState::MouseOver, GMVec4(0, 0, 0, 1.f));

	d->fillStyle.setTexture(GMWidgetResourceManager::Skin, widget->getArea(GMTextureArea::ButtonFillArea));
	d->fillStyle.setFont(0);
	d->fillStyle.setTextureColor(GMControlState::MouseOver, GMVec4(1.f, 1.f, 1.f, .63f));
	d->fillStyle.setTextureColor(GMControlState::Pressed, GMVec4(1.f, 1.f, 1.f, .24f));
	d->fillStyle.setTextureColor(GMControlState::Focus, GMVec4(1.f, 1.f, 1.f, .18f));
	d->fillStyle.setFontColor(GMControlState::Normal, GMVec4(.3f, .3f, .3f, 1.f));
	d->fillStyle.setFontColor(GMControlState::MouseOver, GMVec4(0, 0, 0, 1.f));
}

void GMControlBorder::render(GMfloat elapsed)
{
	if (!getVisible())
		return;

	D(d);
	D_BASE(db, Base);
	d->borderStyle.getTextureColor().blend(GMControlState::Normal, elapsed);
	GMWidget* widget = getParent();
	widget->drawBorder(d->borderStyle, d->corner, db->boundingBox, .8f);
}

void GMControlBorder::setCorner(const GMRect& corner)
{
	D(d);
	d->corner = corner;
}

void GMControlBorder::initStyles()
{
	D(d);
	GMWidget* widget = getParent();
	d->borderStyle.setTexture(GMWidgetResourceManager::Border, widget->getArea(GMTextureArea::BorderArea));
	d->borderStyle.setTextureColor(GMControlState::Normal, GMVec4(1.f, 1.f, 1.f, 1.f));
}