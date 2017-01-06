#ifndef __GAMEOBJECT_PRIVATE_H__
#define __GAMEOBJECT_PRIVATE_H__
#include "common.h"
#include "gmdatacore/object.h"
#include "utilities/autoptr.h"
#include "btBulletCollisionCommon.h"
BEGIN_NS

class GameWorld;
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
};

END_NS
#endif