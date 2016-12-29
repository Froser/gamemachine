#include "stdafx.h"
#include "gameobjectprivate.h"
#include "gmgl/gmglgraphic_engine.h"

GameObjectPrivate::GameObjectPrivate()
	: m_mass(0)
	, m_isDynamic(true)
	, m_localInertia(0, 0, 0)
	, m_world(nullptr)
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

void GameObjectPrivate::setObject(AUTORELEASE Object* obj)
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