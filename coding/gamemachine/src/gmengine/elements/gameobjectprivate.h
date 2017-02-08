#ifndef __GAMEOBJECT_PRIVATE_H__
#define __GAMEOBJECT_PRIVATE_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "utilities/autoptr.h"
#include "btBulletCollisionCommon.h"
#include "gmengine/controller/animation.h"
#include "gmengine/controller/script.h"
BEGIN_NS

#define PARAM(type, name)	\
	bool name##_flag : 1;	\
	type name;
#define PARAM_F(name) PARAM(GMfloat, name)

struct Frictions
{
	PARAM_F(friction);
	PARAM_F(rollingFriction);
	PARAM_F(spinningFriction);
};

enum AnimationState
{
	Stopped,
	Running,
};

struct EventItem
{
	enum Type
	{
		Region,
		
		EventItemEnd,
	};

	struct Action
	{
		std::string name;
		Arguments args;
	};

	Type condition;
	GameObject* targetObject;
	std::vector<Action> actions;
};

struct IEventPredicator
{
	virtual ~IEventPredicator() { }
	virtual bool eventPredicate(GameObject* source, EventItem& item) = 0;
};


class GameWorld;
struct GameObjectPrivate
{
	GameObjectPrivate();
	~GameObjectPrivate();
	void setupPredicator();

	GMuint id;
	btVector3 localScaling;
	btTransform transform;
	btScalar mass;
	bool isDynamic;
	btVector3 localInertia;
	AutoPtr<btCollisionShape> collisionShape;
	btCollisionObject* collisionObject;
	AutoPtr<btMotionState> motionState;
	AutoPtr<Object> object;
	GameWorld* world;
	Frictions frictions;
	Keyframes keyframesRotation;
	Keyframes keyframesTranslation;
	Keyframes keyframesScaling;
	GMint animationStartTick;
	GMint animationDuration;
	AnimationState animationState;
	std::vector<EventItem> eventItems;
	IEventPredicator* predicators[EventItem::EventItemEnd];
};

END_NS
#endif