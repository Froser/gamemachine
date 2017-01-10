#include "stdafx.h"
#include "gameobjectprivate.h"
#include "gmgl/gmglgraphic_engine.h"

GameObjectPrivate::GameObjectPrivate()
	: m_mass(0)
	, m_isDynamic(true)
	, m_localInertia(0, 0, 0)
	, m_world(nullptr)
	, m_pColObj(nullptr)
	, m_animationStartTick(0)
	, m_animationDuration(0)
	, m_animationState(Stopped)
{
	m_transform.setIdentity();
	memset(&m_frictions, 0, sizeof(Frictions));
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

void GameObjectPrivate::setFrictions(const Frictions& frictions)
{
	m_frictions = frictions;
	setFrictions();
}

void GameObjectPrivate::setFrictions()
{
	if (!m_pColObj)
		return;

	if (m_frictions.friction_flag)
		m_pColObj->setFriction(m_frictions.friction);
	if (m_frictions.rollingFriction_flag)
		m_pColObj->setRollingFriction(m_frictions.rollingFriction);
	if (m_frictions.spinningFriction_flag)
		m_pColObj->setSpinningFriction(m_frictions.spinningFriction);
}