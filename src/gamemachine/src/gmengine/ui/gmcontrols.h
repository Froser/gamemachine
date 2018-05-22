#ifndef __GMCONTROL_H__
#define __GMCONTROL_H__
#include <gmcommon.h>
BEGIN_NS

class GMCanvas;

GM_PRIVATE_OBJECT(GMElement)
{
	GMuint texture = 0;
	GMuint font = 0;
	GMRect rc;
};

class GMElement : public GMObject
{
	DECLARE_PRIVATE(GMElement)
	GM_ALLOW_COPY_DATA(GMElement)

public:
	GMElement() = default;

public:
	void setTexture(GMuint texture, const GMRect& rc);
	void setFont(GMuint font);
	void refresh();
};

enum class GMControlType
{
	ControlButton,
};

GM_PRIVATE_OBJECT(GMControl)
{
	GMControlType type = GMControlType::ControlButton;
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
	virtual void render(float elapsed);

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

	virtual bool setElement(GMuint index, GMElement* element);

protected:
	void updateRect();
};

struct GMElementHolder
{
	GMControlType type;
	GMuint index;
	GMElement element;
};
END_NS
#endif