#include "stdafx.h"
#include "rigidgameobject.h"
#include "btBulletDynamicsCommon.h"

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
	world->addRigidBody(collisionObject);
}
