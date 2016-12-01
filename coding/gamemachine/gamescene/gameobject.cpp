#include "stdafx.h"
#include "gameobject.h"
#include "GL/freeglut.h"

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

GLCubeGameObject::GLCubeGameObject(GMfloat size, const btTransform& position, GMfloat* color)
	: m_extents(size / 2, size / 2, size / 2)
	, m_size(size)
	, m_transform(position)
{
	m_color[0] = color[0];
	m_color[1] = color[1];
	m_color[2] = color[2];
}

btMotionState* GLCubeGameObject::createMotionState()
{
	return new btDefaultMotionState(m_transform);
}

void GLCubeGameObject::drawObject()
{
	glColor3fv(m_color);
	glutSolidCube(m_size);
}

btCollisionShape* GLCubeGameObject::createCollisionShape()
{
	return new btBoxShape(m_extents);
}