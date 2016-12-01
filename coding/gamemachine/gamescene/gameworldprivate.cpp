#include "stdafx.h"
#include "gameworldprivate.h"
#include "gameobject.h"

GameWorldPrivate::GameWorldPrivate()
	: m_gravity(0, -10, 0)
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

void GameWorldPrivate::appendObject(GameObject* obj)
{
	btMotionState* motionState = obj->createMotionState();
	btRigidBody::btRigidBodyConstructionInfo rbInfo(obj->getMass(), motionState, obj->getCollisionShape(), obj->getLocalInertia());
	btRigidBody* rigidObj = new btRigidBody(rbInfo);

	m_dynamicsWorld->addRigidBody(rigidObj);
	m_shapes.push_back(obj);
}