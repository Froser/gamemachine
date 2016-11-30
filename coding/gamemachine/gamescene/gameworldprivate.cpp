#include "stdafx.h"
#include "gameworldprivate.h"

GameWorldPrivate::GameWorldPrivate()
	: m_gravity(0, -10, 0)
{

}

void GameWorldPrivate::setGravity(GMfloat x, GMfloat y, GMfloat z)
{
	m_gravity.setX(x);
	m_gravity.setY(x);
	m_gravity.setZ(z);
}

void GameWorldPrivate::init()
{
	m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration));
	m_overlappingPairCache.reset(new btDbvtBroadphase());
	m_solver.reset(new btSequentialImpulseConstraintSolver);
	m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration));
}