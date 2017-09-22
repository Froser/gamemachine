#ifndef __GMCONTROLGAMEOBJECT_H__
#define __GMCONTROLGAMEOBJECT_H__
#include <gmcommon.h>
#include <gamemachine.h>
BEGIN_NS

enum class GMControlEventType
{
	MouseHover,
	MouseDown,
};

// 2D对象的事件
class GMControlEvent
{
public:
	GMControlEvent(GMControlEventType type);
	GMControlEventType type() { return m_type; }

private:
	GMControlEventType m_type;
};

class GM2DMouseHoverEvent : public GMControlEvent
{
public:
	GM2DMouseHoverEvent(const GMMouseState& ms)
		: GMControlEvent(GMControlEventType::MouseHover)
		, m_state(ms)
	{
	}
	GMMouseState state() { return m_state; }

private:
	GMMouseState m_state;
};

class GM2DMouseDownEvent : public GMControlEvent
{
public:
	enum Button
	{
		Left,
		Right,
		Middle,
	};

public:
	GM2DMouseDownEvent(const GMMouseState& ms)
		: GMControlEvent(GMControlEventType::MouseDown)
		, m_state(ms)
	{
	}

	GMMouseState state() { return m_state; }

public:
	bool buttonDown(Button button);

private:
	GMMouseState m_state;
};

//////////////////////////////////////////////////////////////////////////
GM_PRIVATE_OBJECT(GMControlGameObject)
{
	GMRect geometry{ 0 };
	GMRect clientSize{ 0 };
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

public:
	virtual bool canDeferredRendering() override { return false; }

protected:
	virtual void event(GMControlEvent* e) {}
	virtual bool insideGeometry(GMint x, GMint y);
	virtual void updateUI();

protected:
	static GMRectF toViewportCoord(const GMRect& in);
};


END_NS
#endif