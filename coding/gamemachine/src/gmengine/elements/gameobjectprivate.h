#ifndef __GAMEOBJECT_PRIVATE_H__
#define __GAMEOBJECT_PRIVATE_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "utilities/autoptr.h"
#include "btBulletCollisionCommon.h"
#include "gmengine/controller/animation.h"
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

class GameWorld;
struct GameObjectPrivate
{
	GameObjectPrivate();

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
};

END_NS
#endif