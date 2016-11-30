#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include "utilities/autoptr.h"
#include "btBulletDynamicsCommon.h"
BEGIN_NS

class GameWorldPrivate
{
	friend class GameWorld;

private:
	GameWorldPrivate();

	void init();
	void setGravity(GMfloat x, GMfloat y, GMfloat z);

private:
	btVector3 m_gravity;
	AutoPtr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	AutoPtr<btCollisionDispatcher> m_dispatcher;
	AutoPtr<btBroadphaseInterface> m_overlappingPairCache;
	AutoPtr<btSequentialImpulseConstraintSolver> m_solver;
	AutoPtr<btDiscreteDynamicsWorld> m_dynamicsWorld;
};

END_NS
#endif