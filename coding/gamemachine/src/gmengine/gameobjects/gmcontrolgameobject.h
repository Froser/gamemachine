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
enum class GMAnimateState
{
	Stopped,
	Running,
	Reverting,
};

struct GMAnimationTypes
{
	enum Types
	{
		Scaling,
		MaxType,
	};
};

struct GMAnimationStates_t
{
	GMfloat tick = 0;
	GMfloat p = 0;
	GMfloat duration = 0;
	GMfloat start[3] = { 0 };
	GMfloat end[3] = { 0 };
	GMint direction = 1;
	GMAnimateState state = GMAnimateState::Stopped;
	GMInterpolation interpolation;
};

GM_PRIVATE_OBJECT(GMControlGameObject)
{
	AUTORELEASE GMGameObject* stencil = nullptr;
	GMControlGameObject* parent = nullptr;
	GMRect geometry{ 0 };
	GMRect clientSize{ 0 };
	bool mouseHovered = false;
	bool stretch = true;
	Vector<GMControlGameObject*> children;

	GMAnimationStates_t animationStates[GMAnimationTypes::MaxType];
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
	virtual bool canDeferredRendering() override { return false; }
	virtual void setScaling(const linear_math::Matrix4x4& scaling) override;
	virtual void draw() override;

public:
	virtual void notifyControl();
	virtual void animateScaleStart(const GMfloat(&end)[3], GMfloat duration, GMInterpolation interpolation = GMInterpolations<3>::linear);
	virtual void animateScaleEnd();

public:
	void createQuadModel(IPrimitiveCreatorShaderCallback* callback, OUT GMModel** model);
	GMAnimateState getAnimationState(GMAnimationTypes::Types type);

protected:
	virtual void event(GMControlEvent* e);
	virtual bool insideGeometry(GMint x, GMint y);
	virtual void updateUI();
	virtual void updateAnimation();

protected:
	static GMRectF toViewportCoord(const GMRect& in);

private:
	void addChild(GMControlGameObject* child);
};


END_NS
#endif