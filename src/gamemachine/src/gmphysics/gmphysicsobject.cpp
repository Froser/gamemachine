#include "stdafx.h"
#include "gmphysicsobject.h"
#include "gmbulletincludes.h"
#include <gmgameobject.h>
#include "gmphysicsshape.h"
#include "gmbullethelper.h"

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
}

void GMRigidPhysicsObject::detachRigidBody()
{
	D(d);
	d->bodyDetached = true;
}