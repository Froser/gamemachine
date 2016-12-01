#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include <vector>
#include "utilities/autoptr.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

BEGIN_NS

class GameObject;
class GameWorldPrivate
{
	friend class GameWorld;

private:
	GameWorldPrivate();
	~GameWorldPrivate();

	void init();
	void setGravity(GMfloat x, GMfloat y, GMfloat z);
	void appendObject(GameObject* obj);

private:
	btVector3 m_gravity;
	AutoPtr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	AutoPtr<btCollisionDispatcher> m_dispatcher;
	AutoPtr<btBroadphaseInterface> m_overlappingPairCache;
	AutoPtr<btSequentialImpulseConstraintSolver> m_solver;
	AutoPtr<btDiscreteDynamicsWorld> m_dynamicsWorld;
	std::vector<GameObject*> m_shapes;
};

END_NS
#endif