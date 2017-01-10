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
class GameObjectPrivate
{
	friend class GameObject;

public:
	GameObjectPrivate();
	~GameObjectPrivate();

public:
	btTransform& getTransform();

private:
	void setMass(btScalar mass);
	void setObject(AUTORELEASE Object* obj);
	void setTransform(const btTransform& transform);
	void setFrictions(const Frictions& frictions);
	void setFrictions();

private:
	btTransform m_transform;
	btScalar m_mass;
	bool m_isDynamic;
	btVector3 m_localInertia;
	AutoPtr<btCollisionShape> m_pColShape;
	btCollisionObject* m_pColObj;
	AutoPtr<btMotionState> m_pMotionState;
	AutoPtr<Object> m_pObject;
	GameWorld* m_world;
	Frictions m_frictions;
	Keyframes m_keyframes;
	GMint m_animationStartTick;
	GMint m_animationDuration;
	AnimationState m_animationState;
};

END_NS
#endif