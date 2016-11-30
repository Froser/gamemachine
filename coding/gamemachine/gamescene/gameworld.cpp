#include "stdafx.h"
#include "gameworld.h"
#include "btBulletDynamicsCommon.h"

void GameWorld::init()
{
	m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration));
	m_overlappingPairCache.reset(new btDbvtBroadphase());
	m_solver.reset(new btSequentialImpulseConstraintSolver);
	m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration));
}