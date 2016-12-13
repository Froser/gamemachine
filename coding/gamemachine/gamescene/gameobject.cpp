#include "stdafx.h"
#include "gameobject.h"
#include "GL/freeglut.h"
#include "btBulletDynamicsCommon.h"

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

void GameObject::setTransform(const btTransform& transform)
{
	dataRef().setTransform(transform);
}

btTransform& GameObject::getTransform()
{
	return dataRef().m_transform;
}

//////////////////////////////////////////////////////////////////////////
// Simple game object:

GLCubeGameObject::GLCubeGameObject(GMfloat size, const btTransform& position, GMfloat* color)
	: m_size(size)
{
	setExtents(btVector3(size / 2, size / 2, size / 2));
	setTransform(position);

	m_color[0] = color[0];
	m_color[1] = color[1];
	m_color[2] = color[2];
}

void GLCubeGameObject::drawObject()
{
	glColor3fv(m_color);
	glutSolidCube(m_size);
}

btCollisionShape* GLCubeGameObject::createCollisionShape()
{
	return new btBoxShape(getExtents());
}

void GLCubeGameObject::setExtents(const btVector3& extents)
{
	m_extents = extents;
}

btVector3& GLCubeGameObject::getExtents()
{
	return m_extents;
}

void GLCubeGameObject::appendObjectToWorld(btDynamicsWorld* world)
{
	btMotionState* motionState = new btDefaultMotionState(getTransform());
	btRigidBody::btRigidBodyConstructionInfo rbInfo(getMass(), motionState, getCollisionShape(), getLocalInertia());
	btRigidBody* rigidObj = new btRigidBody(rbInfo);
	world->addRigidBody(rigidObj);
	setCollisionObject(rigidObj);
}