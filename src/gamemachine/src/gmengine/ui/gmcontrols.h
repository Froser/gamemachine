#ifndef __GMCONTROL_H__
#define __GMCONTROL_H__
#include <gmcommon.h>
BEGIN_NS

class GMCanvas;

enum class GMControlState
{
	Normal,
	Disabled,
	Hidden,
	Focus,
	Mouseover,
	Pressed,

	EndOfControlState,
};

GM_PRIVATE_OBJECT(GMElementBlendColor)
{
	GMVec4 states[(GMuint)GMControlState::EndOfControlState];
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
	void blend(GMControlState state, GMfloat elapsedTime, GMfloat rate = .7f);

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

GM_PRIVATE_OBJECT(GMElement)
{
	GMuint texture = 0;
	GMuint font = 0;
	GMRect rc;
	GMElementBlendColor textureColor;
	GMElementBlendColor fontColor;
};

class GMElement : public GMObject
{
	DECLARE_PRIVATE(GMElement)
	GM_ALLOW_COPY_DATA(GMElement)

public:
	GMElement() = default;

public:
	void setTexture(GMuint texture, const GMRect& rc);
	void setFont(GMuint font, const GMVec4& defaultColor = GMVec4(1, 1, 1, 1));
	void setFontColor(const GMElementBlendColor& color);
	void setTextureColor(const GMElementBlendColor& color);
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
	GMRect rcBoundingBox;
	Vector<GMElement*> elements;
	GMCanvas* canvas = nullptr;

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
	~GMControl();

public:
	// Message handler
	virtual bool handleKeyboard()
	{
		return false;
	}

	virtual bool handleMouse()
	{
		return false;
	}

	virtual bool canHaveFocus()
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

	virtual void refresh();

	virtual void render(float elapsed)
	{

	}

	virtual void setEnabled(bool enabled)
	{
		D(d);
		d->enabled = enabled;
	}

	virtual bool getEnabled()
	{
		D(d);
		return d->enabled;
	}

	virtual void setVisible(bool visible)
	{
		D(d);
		d->visible = visible;
	}

	virtual bool getVisible()
	{
		D(d);
		return d->visible;
	}

	virtual GMControlType getType()
	{
		D(d);
		return d->type;
	}

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

	virtual GMElement* getElement(GMuint index)
	{
		D(d);
		return d->elements[index];
	}

	virtual void setIsDefault(bool isDefault)
	{
		D(d);
		d->isDefault = isDefault;
	}

	virtual bool setElement(GMuint index, GMElement* element);

	virtual bool containsPoint(const GMPoint& point)
	{
		D(d);
		return GM_inRect(d->rcBoundingBox, point);
	}

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

protected:
	void updateRect();
};

struct GMElementHolder
{
	GMControlType type;
	GMuint index;
	GMElement element;
};

GM_PRIVATE_OBJECT(GMControlStatic)
{
	GMString text;
};

class GMControlStatic : public GMControl
{
	DECLARE_PRIVATE_AND_BASE(GMControlStatic, GMControl)

public:
	GMControlStatic(GMCanvas* parent);

public:
	virtual void render(GMfloat elapsed) override;

	virtual bool containsPoint(const GMPoint&) override
	{
		return false;
	}

public:
	inline const GMString& getText() const
	{
		D(d);
		return d->text;
	}

	void setText(const GMString& text);
};
END_NS
#endif