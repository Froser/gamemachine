#include "stdafx.h"
#include "rigidgameobject.h"
#include "btBulletDynamicsCommon.h"

RigidGameObject::RigidGameObject()
	: m_isSensor(false)
{

}

btCollisionObject* RigidGameObject::createCollisionObject()
{
	D(d);
	btMotionState* motionState = new btDefaultMotionState(d.transform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(d.mass, motionState, d.collisionShape.get(), d.localInertia);
	btRigidBody* rigidObj = new btRigidBody(rbInfo);
	return rigidObj;
}

void RigidGameObject::appendThisObjectToWorld(btDynamicsWorld* world)
{
	D(d);
	btRigidBody* collisionObject = static_cast<btRigidBody*>(d.collisionObject);
	if (m_isSensor)
	{
		world->addRigidBody(collisionObject, short(btBroadphaseProxy::SensorTrigger), short(btBroadphaseProxy::CharacterFilter));
	}
	else
	{
		world->addRigidBody(collisionObject);
	}
}