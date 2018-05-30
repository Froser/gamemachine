#ifndef __GMCONTROL_H__
#define __GMCONTROL_H__
#include <gmcommon.h>
#include <gmmessage.h>
BEGIN_NS

class GMCanvas;

struct GMControlState
{
	enum State
	{
		Normal,
		Disabled,
		Hidden,
		Focus,
		MouseOver,
		Pressed,

		EndOfControlState,
	};
};

GM_PRIVATE_OBJECT(GMElementBlendColor)
{
	GMVec4 states[GMControlState::EndOfControlState];
	GMVec4 current;
};

class GMElementBlendColor : public GMObject
{
	DECLARE_PRIVATE(GMElementBlendColor)
	GM_ALLOW_COPY_DATA(GMElementBlendColor)

public:
	GMElementBlendColor() = default;

public:
	void init(const GMVec4& defaultColor, const GMVec4& disabledColor = GMVec4(.5f, .5f, .5f, .78f), const GMVec4& hiddenColor = GMVec4(0));
	void blend(GMControlState::State state, GMfloat elapsedTime, GMfloat rate = .7f);

public:
	inline const GMVec4& getCurrent()
	{
		D(d);
		return d->current;
	}

	inline void setCurrent(const GMVec4& current)
	{
		D(d);
		d->current = current;
	}

	inline GMVec4* getStates()
	{
		D(d);
		return d->states;
	}
};

GM_PRIVATE_OBJECT(GMStyle)
{
	GMuint texture = 0;
	GMuint font = 0;
	GMRect rc;
	GMElementBlendColor textureColor;
	GMElementBlendColor fontColor;
};

class GMStyle : public GMObject
{
	DECLARE_PRIVATE(GMStyle)
	GM_ALLOW_COPY_DATA(GMStyle)

public:
	GMStyle() = default;

public:
	void setTexture(GMuint texture, const GMRect& rc, const GMVec4& defaultTextureColor = GMVec4(1, 1, 1, 1));
	void setFont(GMuint font, const GMVec4& defaultColor = GMVec4(1, 1, 1, 1));
	void setFontColor(GMControlState::State state, const GMVec4& color);
	void setTextureColor(GMControlState::State state, const GMVec4& color);
	void refresh();

public:
	inline GMElementBlendColor& getTextureColor()
	{
		D(d);
		return d->textureColor;
	}

	inline GMElementBlendColor& getFontColor()
	{
		D(d);
		return d->fontColor;
	}

	inline const GMRect& getTextureRect()
	{
		D(d);
		return d->rc;
	}

	inline GMuint getTexture()
	{
		D(d);
		return d->texture;
	}
};

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
	GMCanvas* canvas = nullptr;

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
	GMControl(GMCanvas* canvas);

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

	inline GMCanvas* getParent()
	{
		D(d);
		return d->canvas;
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
	GMControlStatic(GMCanvas* parent);

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
	GMControlButton(GMCanvas* parent);

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