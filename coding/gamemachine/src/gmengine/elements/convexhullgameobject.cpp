#include "stdafx.h"
#include "convexhullgameobject.h"
#include "gmdatacore/object.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

ConvexHullGameObject::ConvexHullGameObject(AUTORELEASE Object* obj)
{
	setObject(obj);
}

btCollisionShape* ConvexHullGameObject::createCollisionShape()
{
	return new btConvexHullShape(getObject()->vertices().data(), getObject()->vertices().size() / 4, sizeof(Object::DataType) * 4 );
}

void ConvexHullGameObject::appendObjectToWorld(btDynamicsWorld* world)
{
	btMotionState* motionState = new btDefaultMotionState(getTransform());
	btRigidBody::btRigidBodyConstructionInfo rbInfo(getMass(), motionState, getCollisionShape(), getLocalInertia());
	btRigidBody* rigidObj = new btRigidBody(rbInfo);
	world->addRigidBody(rigidObj);
	setCollisionObject(rigidObj);
}
