#ifndef __GAMEWORLD_PRIVATE_H__
#define __GAMEWORLD_PRIVATE_H__
#include "common.h"
#include <vector>
#include "utilities/autoptr.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletCollision/CollisionDispatch/btGhostObject.h"

BEGIN_NS

class GameObject;
class Character;
class GameMachine;
class GameLight;
class GameWorldPrivate
{
	friend class GameWorld;

private:
	GameWorldPrivate();
	~GameWorldPrivate();

	void init();
	void setGravity(GMfloat x, GMfloat y, GMfloat z);
	void appendObject(AUTORELEASE GameObject* obj);
	void appendLight(AUTORELEASE GameLight* light);

private:
	AutoPtr<btDefaultCollisionConfiguration> m_collisionConfiguration;
	AutoPtr<btCollisionDispatcher> m_dispatcher;
	AutoPtr<btBroadphaseInterface> m_overlappingPairCache;
	AutoPtr<btSequentialImpulseConstraintSolver> m_solver;
	AutoPtr<btGhostPairCallback> m_ghostPairCallback;
	AutoPtr<btDiscreteDynamicsWorld> m_dynamicsWorld;
	GameMachine* m_gameMachine;
	std::vector<GameObject*> m_shapes;
	std::vector<GameLight*> m_lights;
	Character* m_character;
	GameObject* m_sky;
	GMfloat m_ellapsed;
};

END_NS
#endif