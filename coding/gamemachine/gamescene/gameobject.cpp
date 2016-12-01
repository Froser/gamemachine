#include "stdafx.h"
#include "gameobject.h"

void GameObject::setObject(Object* obj)
{
	dataRef().setObject(obj);
}

Object* GameObject::getObject()
{
	return dataRef().m_pObject;
}

btCollisionShape* GameObject::getCollisionShape()
{
	if (!dataRef().m_pColShape)
		dataRef().m_pColShape.reset(createCollisionShape());
	return dataRef().m_pColShape;
}

void GameObject::setMass(btScalar mass)
{
	dataRef().setMass(mass);
	if (dataRef().m_isDynamic)
		getCollisionShape()->calculateLocalInertia(dataRef().m_mass, dataRef().m_localInertia);
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

//////////////////////////////////////////////////////////////////////////
// Simple game object:

GLCubeGameObject::GLCubeGameObject(GMfloat width, GMfloat height, GMfloat depth, const btTransform& position)
{

}