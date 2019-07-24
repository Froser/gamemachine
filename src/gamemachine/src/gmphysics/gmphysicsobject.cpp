#include "stdafx.h"
#include "gmphysicsobject.h"
#include "gmbulletincludes.h"
#include <gmgameobject.h>
#include "gmphysicsshape.h"

BEGIN_NS

GM_PRIVATE_OBJECT_ALIGNED(GMPhysicsObject)
{
	GMGameObject* gameObject = nullptr;
	GMMotionStates motionStates;
};

GMPhysicsObject::GMPhysicsObject()
{
	GM_CREATE_DATA(GMPhysicsObject);
}

const GMMotionStates& GMPhysicsObject::getMotionStates()
{
	D(d);
	return d->motionStates;
}

GMGameObject* GMPhysicsObject::getGameObject()
{
	D(d);
	return d->gameObject;
}

void GMPhysicsObject::setGameObject(GMGameObject* gameObject)
{
	D(d);
	GM_ASSERT(!d->gameObject);
	d->gameObject = gameObject;
}

void GMPhysicsObject::setMotionStates(const GMMotionStates& motionStates)
{
	D(d);
	d->motionStates = motionStates;
}

GM_PRIVATE_OBJECT_UNALIGNED(GMRigidPhysicsObject)
{
	GMint32 updateRevision = -1;
	btRigidBody* body = nullptr; // btRigidBody在添加到物理世界后，应该由物理世界管理生命周期
	bool bodyDetached = false;
	btDefaultMotionState* motionState = nullptr;
	GMPhysicsShapeAsset shape;
	GMfloat mass = 0;
	GMPhysicsActivationState state = GMPhysicsActivationState::ActiveTag;
};

GMRigidPhysicsObject::GMRigidPhysicsObject()
{
	GM_CREATE_DATA(GMRigidPhysicsObject);
}

GMRigidPhysicsObject::~GMRigidPhysicsObject()
{
	D(d);
	GM_delete(d->motionState);
	if (!d->bodyDetached)
		GM_delete(d->body);
}

bool GMRigidPhysicsObject::isStaticObject() const
{
	D(d);
	GM_ASSERT(d->body);
	return d->body->isStaticObject();
}

bool GMRigidPhysicsObject::isKinematicObject() const
{
	D(d);
	GM_ASSERT(d->body);
	return d->body->isKinematicObject();
}

bool GMRigidPhysicsObject::isStaticOrKinematicObject() const
{
	D(d);
	GM_ASSERT(d->body);
	return d->body->isStaticOrKinematicObject();
}

bool GMRigidPhysicsObject::hasContactResponse() const
{
	D(d);
	GM_ASSERT(d->body);
	return d->body->hasContactResponse();
}

GMPhysicsActivationState GMRigidPhysicsObject::getActivationState()
{
	D(d);
	GM_ASSERT(d->body);
	return (GMPhysicsActivationState)d->body->getActivationState();
}

void GMRigidPhysicsObject::setActivationState(GMPhysicsActivationState state, bool force)
{
	D(d);
	GM_ASSERT(d->body);
	if (force)
		d->body->forceActivationState((GMint32)state);
	else
		d->body->setActivationState((GMint32)state);
}

void GMRigidPhysicsObject::activate(bool force)
{
	D(d);
	GM_ASSERT(d->body);
	d->body->activate(force);
}

GMMat4 GMRigidPhysicsObject::getCenterOfMassTransform()
{
	D(d);
	GM_ASSERT(d->body);
	GMMat4 val;
	d->body->getCenterOfMassTransform().getOpenGLMatrix(ValuePointer(val));
	return val;
}

GMMat4 GMRigidPhysicsObject::getCenterOfMassTransformInversed()
{
	D(d);
	GM_ASSERT(d->body);
	GMMat4 val;
	d->body->getCenterOfMassTransform().inverse().getOpenGLMatrix(ValuePointer(val));
	return val;
}

const GMMotionStates& GMRigidPhysicsObject::getMotionStates()
{
	D(d);
	D_BASE(db, Base);
	if (d->updateRevision != d->body->getUpdateRevisionInternal())
	{
		d->updateRevision = d->body->getUpdateRevisionInternal();
		d->body->getWorldTransform().getOpenGLMatrix(ValuePointer(db->motionStates.transform));

		btVector3 lv = d->body->getLinearVelocity();
		db->motionStates.linearVelocity = GMVec3(lv[0], lv[1], lv[2]);
	}
	return db->motionStates;
}

void GMRigidPhysicsObject::setShape(GMAsset shape)
{
	D(d);
	D_BASE(db, Base);
	GM_ASSERT(d->shape.isEmpty());
	d->shape = shape;

	const GMMat4& translation = db->gameObject->getTranslation();
	btTransform trans;
	trans.setFromOpenGLMatrix(ValuePointer(translation));
	initRigidBody(
		d->mass,
		trans,
		GMVec3(1, 0, 0)
	);
}

void GMRigidPhysicsObject::setMass(GMfloat mass)
{
	D(d);
	d->mass = mass;
}

btRigidBody* GMRigidPhysicsObject::getRigidBody()
{
	D(d);
	GM_ASSERT(d->body);
	return d->body;
}

GMPhysicsShapeAsset GMRigidPhysicsObject::getShape()
{
	D(d);
	GM_ASSERT(!d->shape.isEmpty());
	return d->shape;
}

void GMRigidPhysicsObject::initRigidBody(GMfloat mass, const btTransform& startTransform, const GMVec3& color)
{
	D(d);
	btCollisionShape* shape = d->shape.getPhysicsShape()->getBulletShape();
	bool isDynamic = (mass != 0.f);

	btVector3 localInertia(0, 0, 0);
	if (isDynamic)
		shape->calculateLocalInertia(mass, localInertia);

	if (d->motionState)
		GM_delete(d->motionState);
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

END_NS