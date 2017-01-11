#include "stdafx.h"
#include "gameobject.h"
#include "btBulletDynamicsCommon.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmengine/elements/gameworld.h"

GameObject::GameObject()
{

}

void GameObject::setObject(AUTORELEASE Object* obj)
{
	D(d);
	d.object.reset(obj);
}

Object* GameObject::getObject()
{
	D(d);
	if (!d.object)
		d.object.reset(new Object());
	return d.object;
}

btCollisionShape* GameObject::getCollisionShape()
{
	D(d);
	if (!d.colShape)
		d.colShape.reset(createCollisionShape());
	return d.colShape;
}

btCollisionObject* GameObject::getCollisionObject()
{
	D(d);
	return d.colObj;
}

void GameObject::setCollisionObject(btCollisionObject* obj)
{
	D(d);
	d.colObj = obj;
	setFrictions();
}

void GameObject::setMass(btScalar mass)
{
	D(d);
	d.mass = mass;
	if (d.mass == 0)
		d.isDynamic = false;
	else
		d.isDynamic = true;

	if (d.isDynamic)
	{
		btVector3 localInertia;
		getCollisionShape()->calculateLocalInertia(d.mass, localInertia);
		d.localInertia = localInertia;
	}
}

btScalar GameObject::getMass()
{
	D(d);
	return d.mass;
}

bool GameObject::isDynamic()
{
	D(d);
	return d.isDynamic;
}

btVector3& GameObject::getLocalInertia()
{
	D(d);
	return d.localInertia;
}

void GameObject::setTransform(const btTransform& transform)
{
	D(d);
	d.transform = transform;
}

btTransform& GameObject::getTransform()
{
	D(d);
	return d.transform;
}

void GameObject::setLocalScaling(const btVector3& scale)
{
	btCollisionShape* pShape = getCollisionShape();
	if (pShape)
		pShape->setLocalScaling(scale);
}

void GameObject::setWorld(GameWorld* world)
{
	D(d);
	ASSERT(!d.world);
	d.world = world;
}

GameWorld* GameObject::getWorld()
{
	D(d);
	return d.world;
}

void GameObject::getReadyForRender(DrawingList& list)
{
	D(d);
	btCollisionObject* obj = getCollisionObject();
	vmath::mat4 M = vmath::mat4::identity();

	if (obj)
	{
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body)
		{
			btTransform trans;
			body->getMotionState()->getWorldTransform(trans);

			btScalar glTrans[16];
			trans.getOpenGLMatrix(glTrans);

			btVector3 scaling = obj->getCollisionShape()->getLocalScaling();
			M = getScalingAndTransformMatrix(glTrans, scaling);
		}

		if (d.animationState == Running)
			M = M * getAnimationMatrix();

		DrawingItem item;
		memcpy(item.trans, M, sizeof(M));
		item.gameObject = this;
		list.push_back(item);
	}
}

vmath::mat4 GameObject::getScalingAndTransformMatrix(btScalar glTrans[16], const btVector3& scaling)
{
	vmath::mat4 T(
		vmath::vec4(glTrans[0], glTrans[1], glTrans[2], glTrans[3]),
		vmath::vec4(glTrans[4], glTrans[5], glTrans[6], glTrans[7]),
		vmath::vec4(glTrans[8], glTrans[9], glTrans[10], glTrans[11]),
		vmath::vec4(glTrans[12], glTrans[13], glTrans[14], glTrans[15])
	);
	vmath::mat4 S(
		vmath::vec4(scaling[0], 0, 0, 0),
		vmath::vec4(0, scaling[1], 0, 0),
		vmath::vec4(0, 0, scaling[2], 0),
		vmath::vec4(0, 0, 0, 1)
	);
	return T * S;
}

vmath::mat4 GameObject::getAnimationMatrix()
{
	D(d);
	GMfloat current = d.world->getElapsed();
	GMfloat start = d.animationStartTick;
	GMfloat percentage = current / (start + d.animationDuration);
	if (percentage > 1)
		percentage -= (int)percentage;
	vmath::quaternion rotation = d.keyframes.calculateRotation(percentage);
	return vmath::rotate(rotation[3], vmath::vec3(rotation[0], rotation[1], rotation[2]));
}

void GameObject::setFrictions(const Frictions& frictions)
{
	D(d);
	d.frictions = frictions;
	setFrictions();
}

void GameObject::setFrictions()
{
	D(d);
	if (!d.colObj)
		return;

	if (d.frictions.friction_flag)
		d.colObj->setFriction(d.frictions.friction);
	if (d.frictions.rollingFriction_flag)
		d.colObj->setRollingFriction(d.frictions.rollingFriction);
	if (d.frictions.spinningFriction_flag)
		d.colObj->setSpinningFriction(d.frictions.spinningFriction);
}

Keyframes& GameObject::getKeyframes()
{
	D(d);
	return d.keyframes;
}

void GameObject::startAnimation(GMuint duration)
{
	D(d);
	d.animationStartTick = d.world ? d.world->getElapsed() : 0;
	d.animationDuration = duration;
	d.animationState = Running;
}

void GameObject::stopAnimation()
{
	D(d);
	d.animationState = Stopped;
}