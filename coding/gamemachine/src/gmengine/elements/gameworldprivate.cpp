#include "stdafx.h"
#include "gameworldprivate.h"
#include "gameobject.h"
#include "utilities/assert.h"
#include "gmgl/gmglgraphic_engine.h"
#include <algorithm>

GameWorldPrivate::GameWorldPrivate()
	: m_character(nullptr)
	, m_sky(nullptr)
	, m_ellapsed(0)
{

}

GameWorldPrivate::~GameWorldPrivate()
{
	for (int i = m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		m_dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

	for (auto iter = m_shapes.begin(); iter != m_shapes.end(); iter++)
	{
		delete *iter;
	}

	for (auto iter = m_lights.begin(); iter != m_lights.end(); iter++)
	{
		delete *iter;
	}
}

void GameWorldPrivate::setGravity(GMfloat x, GMfloat y, GMfloat z)
{
	m_dynamicsWorld->setGravity(btVector3(x, y, z));
}

void GameWorldPrivate::init()
{
	m_collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	m_dispatcher.reset(new btCollisionDispatcher(m_collisionConfiguration));
	m_overlappingPairCache.reset(new btDbvtBroadphase());
	m_solver.reset(new btSequentialImpulseConstraintSolver);
	m_ghostPairCallback.reset(new btGhostPairCallback());
	m_dynamicsWorld.reset(new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration));

	m_dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(m_ghostPairCallback);
}

void GameWorldPrivate::appendObject(AUTORELEASE GameObject* obj)
{
	if (std::find(m_shapes.begin(), m_shapes.end(), obj) != m_shapes.end())
		return;

	ObjectPainter* painter = obj->getObject()->getPainter();
	if (painter)
		painter->init();

	obj->appendObjectToWorld(m_dynamicsWorld);
	m_shapes.push_back(obj);
}

void GameWorldPrivate::appendLight(AUTORELEASE GameLight* light)
{
	if (std::find(m_lights.begin(), m_lights.end(), light) != m_lights.end())
		return;

	m_lights.push_back(light);
}