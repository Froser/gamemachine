#include "stdafx.h"
#include "gameobject.h"
#include "GL/freeglut.h"
#include "btBulletDynamicsCommon.h"

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
	getCollisionShape()->setLocalScaling(scale);
}
