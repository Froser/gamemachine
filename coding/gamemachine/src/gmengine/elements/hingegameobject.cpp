#include "stdafx.h"
#include "hingegameobject.h"
#include "BulletCollision\CollisionShapes\btConeShape.h"

HingeGameObject::HingeGameObject(Object* obj)
	: ConvexHullGameObject(obj)
{
}

HingeGameObject::~HingeGameObject()
{
}

void HingeGameObject::appendThisObjectToWorld(btDynamicsWorld* world)
{
	D(d);
	btRigidBody* body = static_cast<btRigidBody*>(d.collisionObject);
	m_constraint.reset(new btHingeConstraint(*(static_cast<btRigidBody*>(d.collisionObject)), d.transform.getOrigin(), btVector3(0, 1, 0)));
	world->addConstraint(m_constraint);
	ConvexHullGameObject::appendThisObjectToWorld(world);
}