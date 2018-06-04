#ifndef __GMCONTROL_H__
#define __GMCONTROL_H__
#include <gmcommon.h>
#include <gmmessage.h>
#include "gmwidget.h"
BEGIN_NS

class GMWidget;

enum class GMControlType
{
	Button,
	Static,
};

GM_PRIVATE_OBJECT(GMControl)
{
	GMControlType type = GMControlType::Button;
	GMuint id = 0;
	GMint x = 0;
	GMint y = 0;
	GMint width = 0;
	GMint height = 0;
	GMRect boundingBox;
	GMWidget* widget = nullptr;

	bool styleInited = false;
	bool enabled = true;
	bool visible = true;
	bool mouseOver = false;
	bool hasFocus = false;
	bool isDefault = false;

	GMint index = 0; //在控件列表中的索引
};

class GMControl : public GMObject
{
	DECLARE_PRIVATE(GMControl)

public:
	GMControl(GMWidget* widget);

public:
	inline void setEnabled(bool enabled)
	{
		D(d);
		d->enabled = enabled;
	}

	inline bool getEnabled()
	{
		D(d);
		return d->enabled;
	}

	inline void setVisible(bool visible)
	{
		D(d);
		d->visible = visible;
	}

	inline bool getVisible()
	{
		D(d);
		return d->visible;
	}

	inline bool getMouseOver()
	{
		D(d);
		return d->mouseOver;
	}

	inline bool hasFocus()
	{
		D(d);
		return d->hasFocus;
	}

	// Message handler
	virtual bool handleKeyboard(GMSystemKeyEvent* event);
	virtual bool handleMouse(GMSystemMouseEvent* event);

	// Events
	virtual bool onKeyDown(GMSystemKeyEvent* event)
	{
		return false;
	}

	virtual bool onKeyUp(GMSystemKeyEvent* event)
	{
		return false;
	}

	virtual bool onMouseMove(GMSystemMouseEvent* event)
	{
		return false;
	}

	virtual bool onMousePress(GMSystemMouseEvent* event)
	{
		return false;
	}

	virtual bool onMouseRelease(GMSystemMouseEvent* event)
	{
		return false;
	}

	virtual bool onMouseDblClick(GMSystemMouseEvent* event)
	{
		return false;
	}

	virtual bool onMouseWheel(GMSystemMouseWheelEvent* event)
	{
		return false;
	}

	virtual void onFocusIn()
	{
		D(d);
		d->hasFocus = true;
	}

	virtual void onFocusOut()
	{
		D(d);
		d->hasFocus = false;
	}

	virtual void onMouseEnter()
	{
		D(d);
		d->mouseOver = true;
	}

	virtual void onMouseLeave()
	{
		D(d);
		d->mouseOver = false;
	}

	virtual void onHotkey()
	{
	}

	virtual bool onInit()
	{
		return true;
	}

	virtual bool canHaveFocus()
	{
		return false;
	}

	virtual void refresh();

	virtual void render(float elapsed) {}

	virtual void setId(GMuint id)
	{
		D(d);
		d->id = id;
	}

	virtual GMuint getId()
	{
		D(d);
		return d->id;
	}

	virtual void setIndex(GMuint index)
	{
		D(d);
		d->index = index;
	}

	virtual void setPosition(GMint x, GMint y)
	{
		D(d);
		d->x = x;
		d->y = y;
		updateRect();
	}

	virtual void setSize(GMint width, GMint height)
	{
		D(d);
		d->width = width;
		d->height = height;
		updateRect();
	}

	virtual void setIsDefault(bool isDefault)
	{
		D(d);
		d->isDefault = isDefault;
	}

	virtual bool containsPoint(const GMPoint& point)
	{
		D(d);
		return GM_inRect(d->boundingBox, point);
	}

	virtual void initStyles() {}

public:
	inline bool isDefault()
	{
		D(d);
		return d->isDefault;
	}

	inline GMWidget* getParent()
	{
		D(d);
		return d->widget;
	}

	inline GMint getIndex()
	{
		D(d);
		return d->index;
	}

	inline GMControlType getType()
	{
		D(d);
		return d->type;
	}

	inline const GMRect& boundingRect()
	{
		D(d);
		return d->boundingBox;
	}

protected:
	void updateRect();
};

GM_PRIVATE_OBJECT(GMControlStatic)
{
	GMString text;
	GMStyle foreStyle;
};

class GMControlStatic : public GMControl
{
	DECLARE_PRIVATE_AND_BASE(GMControlStatic, GMControl)

public:
	GMControlStatic(GMWidget* parent);

public:
	virtual void render(GMfloat elapsed) override;
	virtual void refresh() override;

	virtual bool containsPoint(const GMPoint&) override
	{
		return false;
	}

protected:
	virtual void initStyles() override;

public:
	inline const GMString& getText() const
	{
		D(d);
		return d->text;
	}

	void setText(const GMString& text);
};

GM_PRIVATE_OBJECT(GMControlButton)
{
	bool pressed = false;
	GMStyle fillStyle;
};

class GMControlButton : public GMControlStatic
{
	DECLARE_PRIVATE_AND_BASE(GMControlButton, GMControlStatic)

public:
	GM_DECLARE_SIGNAL(click);
	
public:
	GMControlButton(GMWidget* parent);

public:
	virtual void refresh() override;

	// virtual void onHotkey
	virtual bool onMousePress(GMSystemMouseEvent* event) override;
	virtual bool onMouseDblClick(GMSystemMouseEvent* event) override;
	virtual bool onMouseRelease(GMSystemMouseEvent* event) override;
	virtual bool containsPoint(const GMPoint& pt) override;
	virtual bool canHaveFocus() override;
	virtual bool onKeyDown(GMSystemKeyEvent* event);
	virtual bool onKeyUp(GMSystemKeyEvent* event);
	virtual void render(GMfloat elapsed) override;

private:
	bool handleMousePressOrDblClick(const GMPoint& pt);
	bool handleMouseRelease(const GMPoint& pt);

protected:
	virtual void initStyles() override;
};
END_NS
#endif