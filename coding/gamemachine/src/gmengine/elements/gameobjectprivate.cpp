#include "stdafx.h"
#include "gameobjectprivate.h"

GameObjectPrivate::GameObjectPrivate()
{
	m_transform.setIdentity();
}

GameObjectPrivate::~GameObjectPrivate()
{
}

void GameObjectPrivate::setMass(btScalar mass)
{
	m_mass = mass;
	if (m_mass == 0)
		m_isDynamic = false;
	else
		m_isDynamic = true;
}

void GameObjectPrivate::setObject(Object* obj)
{
	m_pObject.reset(obj);
}

void GameObjectPrivate::setTransform(const btTransform& transform)
{
	m_transform = transform;
}

btTransform& GameObjectPrivate::getTransform()
{
	return m_transform;
}