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
	Border,
	Static,
};

GM_PRIVATE_OBJECT(GMControl)
{
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
	GM_DECLARE_PRIVATE(GMControl)

public:
	enum StyleType
	{
		StyleBegin = 1,
	};

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

	bool handleKeyboard(GMSystemKeyEvent* event);
	bool handleMouse(GMSystemMouseEvent* event);

	//! 控件第一次处理消息的方法。
	/*!
	  当一个系统消息发送到此控件后，此方法会比处理键盘、鼠标消息函数更早调用。<BR>
	  如果返回true，表示此消息已经被处理，之后的消息函数将不会被调用。
	  \return 是否已经处理此消息。
	*/
	virtual bool msgProc(GMSystemEvent* event);

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

	virtual bool onMouseDown(GMSystemMouseEvent* event)
	{
		return false;
	}

	virtual bool onMouseUp(GMSystemMouseEvent* event)
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

	virtual bool onChar(GMSystemCharEvent* event)
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

	virtual GMStyle& getStyle(StyleType style);

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

	inline const GMRect& boundingRect()
	{
		D(d);
		return d->boundingBox;
	}

	inline GMint getWidth()
	{
		D(d);
		return d->width;
	}

	inline GMint getHeight()
	{
		D(d);
		return d->height;
	}

protected:
	virtual void updateRect();
};

GM_PRIVATE_OBJECT(GMControlStatic)
{
	GMString text;
	GMStyle foreStyle;
};

class GMControlStatic : public GMControl
{
	GM_DECLARE_PRIVATE_AND_BASE(GMControlStatic, GMControl)

public:
	enum StyleType
	{
		ForeStyle = Base::StyleBegin,
	};

public:
	GMControlStatic(GMWidget* widget) : Base(widget) { initStyles(widget); }

public:
	virtual void render(GMfloat elapsed) override;
	virtual void refresh() override;
	virtual GMStyle& getStyle(Base::StyleType style) override;

	virtual bool containsPoint(const GMPoint&) override
	{
		return false;
	}

private:
	void initStyles(GMWidget* widget);

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
	GM_DECLARE_PRIVATE_AND_BASE(GMControlButton, GMControlStatic)

public:
	enum StyleType
	{
		ForeStyle = Base::StyleBegin,
		FillStyle,
	};

public:
	GM_DECLARE_SIGNAL(click);
	
public:
	GMControlButton(GMWidget* widget) : Base(widget) { initStyles(widget); }

public:
	virtual void refresh() override;
	virtual GMStyle& getStyle(GMControl::StyleType style) override;

	// virtual void onHotkey
	virtual bool onMouseDown(GMSystemMouseEvent* event) override;
	virtual bool onMouseDblClick(GMSystemMouseEvent* event) override;
	virtual bool onMouseUp(GMSystemMouseEvent* event) override;
	virtual bool containsPoint(const GMPoint& pt) override;
	virtual bool canHaveFocus() override;
	virtual bool onKeyDown(GMSystemKeyEvent* event);
	virtual bool onKeyUp(GMSystemKeyEvent* event);
	virtual void render(GMfloat elapsed) override;

private:
	bool handleMousePressOrDblClick(const GMPoint& pt);
	bool handleMouseRelease(const GMPoint& pt);
	void initStyles(GMWidget* widget);
};


GM_PRIVATE_OBJECT(GMControlBorder)
{
	GMStyle borderStyle;
	GMRect corner;
};

class GMControlBorder : public GMControl
{
	GM_DECLARE_PRIVATE_AND_BASE(GMControlBorder, GMControl);

public:
	GMControlBorder(GMWidget* widget) : Base(widget) { initStyles(widget); }

public:
	virtual void render(GMfloat elapsed) override;
	virtual bool containsPoint(const GMPoint& point) override;

public:
	void setCorner(const GMRect& corner);

private:
	void initStyles(GMWidget* widget);
};
END_NS
#endif