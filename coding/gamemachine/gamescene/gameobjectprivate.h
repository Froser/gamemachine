#ifndef __GAMEOBJECT_PRIVATE_H__
#define __GAMEOBJECT_PRIVATE_H__
#include "common.h"
#include "core/objstruct.h"
#include "utilities/autoptr.h"
#include "btBulletCollisionCommon.h"
BEGIN_NS

class GameObjectPrivate
{
	friend class GameObject;

public:
	GameObjectPrivate();
	~GameObjectPrivate();

private:
	void setMass(btScalar mass);
	void setObject(Object* obj);

private:
	btTransform m_transform;
	btScalar m_mass;
	bool m_isDynamic;
	btVector3 m_localInertia;
	AutoPtr<btCollisionShape> m_pColShape;
	AutoPtr<btMotionState> m_pMotionState;
	AutoPtr<Object> m_pObject;
};

END_NS
#endif