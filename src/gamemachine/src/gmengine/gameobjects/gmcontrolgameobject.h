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
	GMControlGameObject* parent = nullptr;
	GMRect geometry{ 0 };
	GMfloat geometryScaling[2] = { 1, 1 };
	GMRect clientSize{ 0 };
	bool mouseHovered = false;
	bool stretch = true;
	Vector<GMControlGameObject*> children;
};

class GMControlGameObject : public GMGameObject
{
	DECLARE_PRIVATE_AND_BASE(GMControlGameObject, GMGameObject)

public:
	GMControlGameObject(GMControlGameObject* parent = nullptr);
	~GMControlGameObject();

public:
	inline void setParent(GMControlGameObject* parent) { D(d); d->parent = parent; }
	inline const GMRect& getGeometry() { D(d); return d->geometry; }
	inline void setGeometry(const GMRect& rect) { D(d); d->geometry = rect; updateGeometry(); }
	inline void setWidth(GMint width) { D(d); d->geometry.width = width; updateGeometry();  }
	inline void setHeight(GMint height) { D(d); d->geometry.height = height; updateGeometry();  }

public:
	virtual void onAppendingObjectToWorld() override;
	virtual bool canDeferredRendering() override { return false; }
	virtual void setScaling(const GMMat4& scaling) override;
	virtual void setTranslation(const GMMat4& translation) override;
	virtual void setRotation(const GMQuat& rotation) override;

public:
	virtual void notifyControl();

public:
	void createQuadModel(IPrimitiveCreatorShaderCallback* callback, OUT GMModel** model);

protected:
	virtual void event(GMControlEvent* e);
	virtual bool insideGeometry(GMint x, GMint y);
	virtual void updateUI();
	virtual void updateGeometry();

public:
	static GMRectF toViewportCoord(const GMRect& in);
	static GMRect toControlCoord(const GMRectF& in);

private:
	void addChild(GMControlGameObject* child);
	void scalingGeometry(const GMMat4& scaling);
	void translateGeometry(const GMMat4& scaling);
};


END_NS
#endif
