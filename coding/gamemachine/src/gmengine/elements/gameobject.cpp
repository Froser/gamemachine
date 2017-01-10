#include "stdafx.h"
#include "gameobject.h"
#include "btBulletDynamicsCommon.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmengine/elements/gameworld.h"

void GameObject::setObject(AUTORELEASE Object* obj)
{
	dataRef().setObject(obj);
}

Object* GameObject::getObject()
{
	if (!dataRef().m_pObject)
		dataRef().m_pObject.reset(new Object());
	return dataRef().m_pObject;
}

btCollisionShape* GameObject::getCollisionShape()
{
	if (!dataRef().m_pColShape)
		dataRef().m_pColShape.reset(createCollisionShape());
	return dataRef().m_pColShape;
}

btCollisionObject* GameObject::getCollisionObject()
{
	return dataRef().m_pColObj;
}

void GameObject::setCollisionObject(btCollisionObject* obj)
{
	dataRef().m_pColObj = obj;
	dataRef().setFrictions();
}

void GameObject::setMass(btScalar mass)
{
	dataRef().setMass(mass);
	if (dataRef().m_isDynamic)
	{
		btVector3 localInertia;
		getCollisionShape()->calculateLocalInertia(dataRef().m_mass, localInertia);
		dataRef().m_localInertia = localInertia;
	}
}

btScalar GameObject::getMass()
{
	return dataRef().m_mass;
}

bool GameObject::isDynamic()
{
	return dataRef().m_isDynamic;
}

btVector3& GameObject::getLocalInertia()
{
	return dataRef().m_localInertia;
}

void GameObject::setTransform(const btTransform& transform)
{
	dataRef().setTransform(transform);
}

btTransform& GameObject::getTransform()
{
	return dataRef().m_transform;
}

void GameObject::setLocalScaling(const btVector3& scale)
{
	btCollisionShape* pShape = getCollisionShape();
	if (pShape)
		pShape->setLocalScaling(scale);
}

void GameObject::setWorld(GameWorld* world)
{
	ASSERT(!dataRef().m_world);
	dataRef().m_world = world;
}

GameWorld* GameObject::getWorld()
{
	return dataRef().m_world;
}

void GameObject::getReadyForRender(DrawingList& list)
{
	btCollisionObject* obj = getCollisionObject();
	btRigidBody* body = btRigidBody::upcast(obj);
	if (body)
	{
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);

		btScalar glTrans[16];
		trans.getOpenGLMatrix(glTrans);

		btVector3 scaling = obj->getCollisionShape()->getLocalScaling();
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

		vmath::mat4 M = T * S;

		if (dataRef().m_animationState == Running)
		{
			GMfloat current = dataRef().m_world->getElapsed();
			GMfloat start = dataRef().m_animationStartTick;
			GMfloat percentage = current / (start + dataRef().m_animationDuration);
			if (percentage > 1)
				percentage -= (int)percentage;
			vmath::quaternion rotation = dataRef().m_keyframes.calculateRotation(percentage);
			M = M * vmath::rotate(rotation[3], vmath::vec3(rotation[0], rotation[1], rotation[2]));
		}

		DrawingItem item;
		memcpy(item.trans, M, sizeof(T));
		item.gameObject = this;
		list.push_back(item);
	}
}

void GameObject::setFrictions(const Frictions& frictions)
{
	dataRef().setFrictions(frictions);
}

Keyframes& GameObject::getKeyframes()
{
	return dataRef().m_keyframes;
}

void GameObject::startAnimation(GMuint duration)
{
	dataRef().m_animationStartTick = dataRef().m_world ? dataRef().m_world->getElapsed() : 0;
	dataRef().m_animationDuration = duration;
	dataRef().m_animationState = Running;
}

void GameObject::stopAnimation()
{
	dataRef().m_animationState = Stopped;
}