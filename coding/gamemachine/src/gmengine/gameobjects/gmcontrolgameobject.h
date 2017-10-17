#ifndef __GMCONTROLGAMEOBJECT_H__
#define __GMCONTROLGAMEOBJECT_H__
#include <gmcommon.h>
#include <gamemachine.h>
BEGIN_NS

enum class GMControlEventType
{
	MouseHover,
	MouseLeave,
	MouseDown,
};

// 2D对象的事件
class GMControlEvent
{
public:
	GMControlEvent(GMControlEventType type, GMEventName eventName);
	GMControlEventType type() { return m_type; }

public:
	GMEventName getEventName() { return m_eventName; }

private:
	GMEventName m_eventName;
	GMControlEventType m_type;
};

class GM2DMouseEvent : public GMControlEvent
{
public:
	GM2DMouseEvent(GMEventName eventName, GMControlEventType eventType, const GMMouseState& ms)
		: GMControlEvent(eventType, eventName)
		, m_state(ms)
	{
	}
	GMMouseState state() { return m_state; }

protected:
	GMMouseState m_state;
};

class GM2DMouseHoverEvent : public GM2DMouseEvent
{
public:
	GM2DMouseHoverEvent(const GMMouseState& ms)
		: GM2DMouseEvent(GM_CONTROL_EVENT_ENUM(MouseHover), GMControlEventType::MouseHover, ms)
	{
	}
};

class GM2DMouseLeaveEvent : public GM2DMouseEvent
{
public:
	GM2DMouseLeaveEvent(const GMMouseState& ms)
		: GM2DMouseEvent(GM_CONTROL_EVENT_ENUM(MouseLeave), GMControlEventType::MouseLeave, ms)
	{
	}
};

class GM2DMouseDownEvent : public GM2DMouseEvent
{
public:
	enum Button
	{
		Left,
		Right,
		Middle,
	};

	GM2DMouseDownEvent(const GMMouseState& ms)
		: GM2DMouseEvent(GM_CONTROL_EVENT_ENUM(MouseDown), GMControlEventType::MouseDown, ms)
	{
	}

public:
	bool buttonDown(Button button);
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(GMControlGameObject)
{
	GMRect geometry{ 0 };
	GMRect clientSize{ 0 };
	bool mouseHovered = false;
	bool stretch = true;
};

class GMControlGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMControlGameObject)

	GM_DECLARE_PROPERTY(Stretch, stretch, bool);

public:
	GMControlGameObject();

public:
	void setGeometry(const GMRect& rect);

public:
	virtual void notifyControl();
	virtual bool canDeferredRendering() override { return false; }

protected:
	virtual void event(GMControlEvent* e);
	virtual bool insideGeometry(GMint x, GMint y);
	virtual void updateUI();

protected:
	static GMRectF toViewportCoord(const GMRect& in);
};


END_NS
#endif