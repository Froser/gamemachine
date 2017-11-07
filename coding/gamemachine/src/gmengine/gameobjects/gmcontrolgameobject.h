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
struct IPrimitiveCreatorShaderCallback;
GM_PRIVATE_OBJECT(GMControlGameObject)
{
	AUTORELEASE GMGameObject* stencil = nullptr;
	GMControlGameObject* parent = nullptr;
	GMRect geometry{ 0 };
	GMRect clientSize{ 0 };
	bool mouseHovered = false;
	bool stretch = true;
	Vector<GMControlGameObject*> children;
};

class GMControlGameObject : public GMGameObject
{
	DECLARE_PRIVATE(GMControlGameObject)

	typedef GMGameObject Base;

	GM_DECLARE_PROPERTY(Stretch, stretch, bool);

public:
	GMControlGameObject(GMControlGameObject* parent = nullptr);
	~GMControlGameObject();

public:
	inline void setParent(GMControlGameObject* parent) { D(d); d->parent = parent; }
	inline GMGameObject* getStencil() { D(d); return d->stencil; }
	inline const GMRect& getGeometry() { D(d); return d->geometry; }
	inline void setGeometry(const GMRect& rect) { D(d); d->geometry = rect; }
	inline void setWidth(GMint width) { D(d); d->geometry.width = width; }
	inline void setHeight(GMint height) { D(d); d->geometry.height = height; }

public:
	virtual void onAppendingObjectToWorld() override;

public:
	virtual void notifyControl();
	virtual bool canDeferredRendering() override { return false; }

public:
	void createQuadModel(IPrimitiveCreatorShaderCallback* callback, OUT GMModel** model);

protected:
	virtual void event(GMControlEvent* e);
	virtual bool insideGeometry(GMint x, GMint y);
	virtual void updateUI();

protected:
	static GMRectF toViewportCoord(const GMRect& in);

private:
	void addChild(GMControlGameObject* child);
};


END_NS
#endif