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

GM_PRIVATE_CLASS(GMControl);
class GM_EXPORT GMControl : public GMObject
{
	GM_DECLARE_PRIVATE(GMControl)

public:
	enum StyleType
	{
		StyleBegin = 1,
	};

public:
	GMControl(GMWidget* widget);
	~GMControl();

public:
	void setEnabled(bool enabled);
	bool getEnabled();
	void setVisible(bool visible);
	bool getVisible();
	bool getMouseOver();
	bool hasFocus();
	bool isDefault() GM_NOEXCEPT;
	GMWidget* getParent() GM_NOEXCEPT;
	GMint32 getIndex() GM_NOEXCEPT;
	const GMRect& getBoundingRect() GM_NOEXCEPT;
	GMint32 getWidth() GM_NOEXCEPT;
	GMint32 getHeight() GM_NOEXCEPT;
	GMPoint toControlSpace(const GMPoint& rc) GM_NOEXCEPT;
	void setPositionFlag(GMControlPositionFlag flag) GM_NOEXCEPT;
	GMControlPositionFlag getPositionFlag() const GM_NOEXCEPT;

public:
	virtual bool handleKeyboard(GMSystemKeyEvent* event);
	virtual bool handleMouse(GMSystemMouseEvent* event);

	//! 控件第一次处理消息的方法。
	/*!
	  当一个系统消息发送到此控件后，此方法会比处理键盘、鼠标消息函数更早调用。<BR>
	  如果返回true，表示此消息已经被处理，之后的消息函数将不会被调用。
	  \return 是否已经处理此消息。
	*/
	virtual bool handleSystemEvent(GMSystemEvent* event);

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

	virtual bool onCaptureChanged(GMSystemCaptureChangedEvent* event)
	{
		return false;
	}

	virtual void onFocusIn();

	virtual void onFocusOut();

	virtual void onMouseEnter();

	virtual void onMouseLeave();

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

	virtual bool containsPoint(GMPoint point);

	virtual void setIndex(GMint32 index);

	virtual void setPosition(GMint32 x, GMint32 y);

	virtual void setSize(GMint32 width, GMint32 height);

	virtual void setIsDefault(bool isDefault);

protected:
	virtual void updateRect();
};

GM_PRIVATE_CLASS(GMControlLabel);
class GM_EXPORT GMControlLabel : public GMControl
{
	GM_DECLARE_PRIVATE(GMControlLabel)
	GM_DECLARE_BASE(GMControl);

public:
	enum StyleType
	{
		ForeStyle = Base::StyleBegin,
	};

public:
	static GMControlLabel* createControl(
		GMWidget* widget,
		const GMString& text,
		const GMVec4& fontColor,
		GMint32 x,
		GMint32 y,
		GMint32 width,
		GMint32 height,
		bool isDefault
	);

protected:
	GMControlLabel(GMWidget* widget);

public:
	~GMControlLabel();

public:
	virtual void render(GMDuration elapsed) override;
	virtual void refresh() override;
	virtual GMStyle& getStyle(Base::StyleType style) override;
	virtual bool containsPoint(GMPoint) override
	{
		return false;
	}

private:
	void initStyles(GMWidget* widget);

public:
	const GMString& getText() const;
	void setText(const GMString& text);
	void setFontColor(const GMVec4& color);
};

GM_PRIVATE_CLASS(GMControlButton);
class GM_EXPORT GMControlButton : public GMControlLabel
{
	GM_DECLARE_PRIVATE(GMControlButton)
	GM_DECLARE_BASE(GMControlLabel)

public:
	enum StyleType
	{
		ForeStyle = Base::StyleBegin,
		FillStyle,
	};

	GM_DECLARE_SIGNAL(click);
	
public:
	static GMControlButton* createControl(
		GMWidget* widget,
		const GMString& text,
		GMint32 x,
		GMint32 y,
		GMint32 width,
		GMint32 height,
		bool isDefault
	);

protected:
	GMControlButton(GMWidget* widget);

public:
	virtual void refresh() override;
	virtual GMStyle& getStyle(GMControl::StyleType style) override;

	// virtual void onHotkey
	virtual bool onMouseDown(GMSystemMouseEvent* event) override;
	virtual bool onMouseDblClick(GMSystemMouseEvent* event) override;
	virtual bool onMouseUp(GMSystemMouseEvent* event) override;
	virtual bool canHaveFocus() override;
	virtual bool onKeyDown(GMSystemKeyEvent* event);
	virtual bool onKeyUp(GMSystemKeyEvent* event);
	virtual bool containsPoint(GMPoint point) override;
	virtual void render(GMDuration elapsed) override;

private:
	bool handleMousePressOrDblClick(const GMPoint& pt);
	bool handleMouseRelease(const GMPoint& pt);
	void initStyles(GMWidget* widget);
};

GM_PRIVATE_CLASS(GMControlBorder);
class GMControlBorder : public GMControl
{
	GM_DECLARE_PRIVATE(GMControlBorder);
	GM_DECLARE_BASE(GMControl)
	GM_DECLARE_PROPERTY(GMRect, Corner);
	GM_DECLARE_PROPERTY(GMStyle, BorderStyle);

public:
	static GMControlBorder* createControl(
		GMWidget* widget,
		GMint32 x,
		GMint32 y,
		GMint32 width,
		GMint32 height,
		const GMRect& cornerRect
	);

protected:
	GMControlBorder(GMWidget* widget);

public:
	virtual void render(GMDuration elapsed) override;
	virtual bool containsPoint(GMPoint point) override;

private:
	void initStyles(GMWidget* widget);
};

enum class GMControlScrollBarArrowState
{
	Clear,
	ClickedUp,
	ClickedDown,
	HeldUp,
	HeldDown,
	TrackUp,
	TrackDown,
	TrackHeldUp,
	TrackHeldDown,
};

GM_PRIVATE_CLASS(GMControlScrollBar);
class GM_EXPORT GMControlScrollBar : public GMControl
{
	GM_DECLARE_PRIVATE(GMControlScrollBar)
	GM_DECLARE_BASE(GMControl)
	GM_DECLARE_PROPERTY(GMint32, PageStep)
	GM_DECLARE_PROPERTY(GMint32, SingleStep)
	GM_DECLARE_GETTER_ACCESSOR(GMint32, Value, public)
	GM_DECLARE_GETTER_ACCESSOR(GMint32, Maximum, public)
	GM_DECLARE_GETTER_ACCESSOR(GMint32, Minimum, public)
	GM_DECLARE_PROPERTY(bool, CanRequestFocus)

	GM_DECLARE_SIGNAL(valueChanged)
	GM_DECLARE_SIGNAL(startDragThumb)
	GM_DECLARE_SIGNAL(endDragThumb)

public:
	enum StyleType
	{
		ArrowUp = Base::StyleBegin,
		ArrowDown,
		Track,
		Thumb,
	};

public:
	static GMControlScrollBar* createControl(
		GMWidget* widget,
		GMint32 x,
		GMint32 y,
		GMint32 width,
		GMint32 height,
		bool isDefault
	);

	~GMControlScrollBar();

protected:
	GMControlScrollBar(GMWidget* widget);

public:
	void setMaximum(GMint32 maximum);
	void setMinimum(GMint32 minimum);
	void setValue(GMint32 value);
	void setThumbCorner(const GMRect& corner);

protected:
	virtual void updateRect() override;
	virtual bool canHaveFocus() override;
	virtual GMStyle& getStyle(GMControl::StyleType style) override;
	virtual bool onMouseDblClick(GMSystemMouseEvent* event) override;
	virtual bool onMouseMove(GMSystemMouseEvent* event) override;
	virtual bool onMouseDown(GMSystemMouseEvent* event) override;
	virtual bool onMouseUp(GMSystemMouseEvent* event) override;

// Events
	virtual bool onCaptureChanged(GMSystemCaptureChangedEvent* event) override;

public:
	virtual void render(GMDuration elapsed) override;
	virtual bool handleMouse(GMSystemMouseEvent* event) override;

private:
	void initStyles(GMWidget* widget);
	void updateThumbRect();
	bool handleMouseClick(GMSystemMouseEvent* event);

protected:
	void clampValue();
	void scroll(GMint32 value);
};

END_NS
#endif