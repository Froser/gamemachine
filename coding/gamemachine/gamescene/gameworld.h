#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__
#include "common.h"
#include "utilities/autoptr.h"
#include "gameworldprivate.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

BEGIN_NS

class GameWorld
{
	DEFINE_PRIVATE(GameWorld)

public:
	void init();

private:
	AutoPtr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	AutoPtr<btCollisionDispatcher> m_dispatcher;
	AutoPtr<btBroadphaseInterface> m_overlappingPairCache;
	AutoPtr<btSequentialImpulseConstraintSolver> m_solver;
	AutoPtr<btDiscreteDynamicsWorld> m_dynamicsWorld;
};

END_NS
#endif