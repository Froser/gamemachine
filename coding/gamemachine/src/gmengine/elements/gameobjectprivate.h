#ifndef __GAMEOBJECT_PRIVATE_H__
#define __GAMEOBJECT_PRIVATE_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "utilities/autoptr.h"
#include "btBulletCollisionCommon.h"
#include "../controller/animation.h"
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
struct Frictions;
struct GameObjectPrivate
{
	GameObjectPrivate();

	btTransform transform;
	btScalar mass;
	bool isDynamic;
	btVector3 localInertia;
	AutoPtr<btCollisionShape> colShape;
	btCollisionObject* colObj;
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