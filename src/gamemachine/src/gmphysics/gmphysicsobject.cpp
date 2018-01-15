#include "stdafx.h"
#include "gmphysicsobject.h"
#include "gmbulletincludes.h"
#include <gmgameobject.h>

GMRigidPhysicsObject::~GMRigidPhysicsObject()
{
	D(d);
	GM_delete(d->shape);
}

void GMRigidPhysicsObject::setMass(GMfloat mass)
{
	D(d);
	d->mass = mass;
}

void GMRigidPhysicsObject::initAsBoxShape(const glm::vec3& halfExtents)
{
	D(d);
	D_BASE(db, Base);
	d->type = GMShapeType::Box;
	d->shape = new btBoxShape(btVector3(halfExtents[0], halfExtents[1], halfExtents[2]));

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
	GM_ASSERT((!d->shape || d->shape->getShapeType() != INVALID_SHAPE_PROXYTYPE));
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		d->shape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* motionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo cInfo(mass, motionState, d->shape, localInertia);

	d->body = new btRigidBody(cInfo);
}