#include "stdafx.h"
#include "gmphysicsobject.h"
#include "gmbulletincludes.h"
#include <gmgameobject.h>
#include "gmphysicsshape.h"
#include "gmbullethelper.h"

const GMMotionStates& GMPhysicsObject::getMotionStates()
{
	D(d);
	return d->motionStates;
}

GMRigidPhysicsObject::~GMRigidPhysicsObject()
{
	D(d);
	GM_delete(d->motionState);
	if (!d->bodyDetached)
		GM_delete(d->body);
}

void GMRigidPhysicsObject::setMass(GMfloat mass)
{
	D(d);
	d->mass = mass;
}

const GMMotionStates& GMRigidPhysicsObject::getMotionStates()
{
	D(d);
	D_BASE(db, Base);
	if (d->updateRevision != d->body->getUpdateRevisionInternal())
	{
		d->updateRevision = d->body->getUpdateRevisionInternal();
		d->body->getWorldTransform().getOpenGLMatrix(glm::value_ptr(db->motionStates.transform));

		btVector3 lv = d->body->getLinearVelocity();
		db->motionStates.linearVelocity = glm::vec3(lv[0], lv[1], lv[2]);
	}
	return db->motionStates;
}

void GMRigidPhysicsObject::setShape(GMAsset shape)
{
	D(d);
	D_BASE(db, Base);
	GM_ASSERT(!d->shape);
	d->shape = GMAssets::getPhysicsShape(shape);

	const glm::mat4& translation = db->gameObject->getTranslation();
	btTransform trans;
	trans.setFromOpenGLMatrix(glm::value_ptr(translation));
	initRigidBody(
		d->mass,
		trans,
		glm::vec3(1, 0, 0)
	);
}

void GMRigidPhysicsObject::initRigidBody(GMfloat mass, const btTransform& startTransform, const glm::vec3& color)
{
	D(d);
	btCollisionShape* shape = d->shape->getBulletShape();
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	d->motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, d->motionState, shape, localInertia);

	d->body = new btRigidBody(cInfo);

	// 将GM对象与此bullet对象绑定
	d->body->setUserPointer(this);
}

void GMRigidPhysicsObject::detachRigidBody()
{
	D(d);
	d->bodyDetached = true;
}