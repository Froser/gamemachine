#include "stdafx.h"
#include "gameobject.h"
#include "btBulletDynamicsCommon.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmengine/elements/gameworld.h"

GameObject::GameObject()
{

}

void GameObject::initPhysics(btDynamicsWorld* world)
{
	D(d);
	ASSERT(!d.collisionShape);
	d.collisionShape.reset(createCollisionShape());

	// scaling
	btVector3 scaling = d.localScaling;
	d.collisionShape->setLocalScaling(scaling);

	// mass, localInertia, dynamic
	if (d.mass == 0)
		d.isDynamic = false;
	else
		d.isDynamic = true;

	if (d.isDynamic)
	{
		btVector3 localInertia;
		d.collisionShape->calculateLocalInertia(d.mass, localInertia);
		d.localInertia = localInertia;
	}

	// frictions
	setFrictions();
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

btCollisionObject* GameObject::getCollisionObject()
{
	D(d);
	return d.collisionObject;
}

void GameObject::setMass(btScalar mass)
{
	D(d);
	d.mass = mass;
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

void GameObject::setTransform(const btTransform& transform)
{
	D(d);
	d.transform = transform;
}

void GameObject::setLocalScaling(const btVector3& scale)
{
	D(d);
	d.localScaling = scale;
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
		{
			AnimationMatrices mat = getAnimationMatrix();
			M = mat.tranlation * M * mat.rotation * mat.scaling;
		}

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

void GameObject::appendObjectToWorld(btDynamicsWorld* world)
{
	D(d);
	initPhysics(world);
	d.collisionObject = createCollisionObject();
	appendThisObjectToWorld(world);
}

AnimationMatrices GameObject::getAnimationMatrix()
{
	D(d);
	GMfloat current = d.world->getElapsed();
	GMfloat start = d.animationStartTick;
	GMfloat percentage = current / (start + d.animationDuration);
	if (percentage > 1)
		percentage -= (int)percentage;

	vmath::quaternion rotation = d.keyframesRotation.isEmpty() ? vmath::quaternion(1, 0, 0, 0) : d.keyframesRotation.calculateInterpolation(percentage, true);
	vmath::quaternion translation = d.keyframesTranslation.isEmpty() ? vmath::quaternion(0, 0, 0, 0) : d.keyframesTranslation.calculateInterpolation(percentage, false);
	vmath::quaternion scaling = d.keyframesScaling.isEmpty() ? vmath::quaternion(1, 1, 1, 1) : d.keyframesScaling.calculateInterpolation(percentage, false);

	AnimationMatrices mat = {
		vmath::rotate(rotation[3], rotation[0], rotation[1], rotation[2]),
		vmath::translate(translation[0], translation[1], translation[2]),
		vmath::scale(scaling[0], scaling[1], scaling[2]),
	};

	return mat;
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
	if (!d.collisionObject)
		return;

	if (d.frictions.friction_flag)
		d.collisionObject->setFriction(d.frictions.friction);
	if (d.frictions.rollingFriction_flag)
		d.collisionObject->setRollingFriction(d.frictions.rollingFriction);
	if (d.frictions.spinningFriction_flag)
		d.collisionObject->setSpinningFriction(d.frictions.spinningFriction);
}

Keyframes& GameObject::getKeyframesRotation()
{
	D(d);
	return d.keyframesRotation;
}

Keyframes& GameObject::getKeyframesTranslation()
{
	D(d);
	return d.keyframesTranslation;
}

Keyframes& GameObject::getKeyframesScaling()
{
	D(d);
	return d.keyframesScaling;
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