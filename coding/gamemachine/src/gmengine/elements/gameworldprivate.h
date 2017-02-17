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
struct GameWorldPrivate
{
	GameWorldPrivate();

	AutoPtr<btDefaultCollisionConfiguration> collisionConfiguration;
	AutoPtr<btCollisionDispatcher> dispatcher;
	AutoPtr<btBroadphaseInterface> overlappingPairCache;
	AutoPtr<btSequentialImpulseConstraintSolver> solver;
	AutoPtr<btGhostPairCallback> ghostPairCallback;
	AutoPtr<btDiscreteDynamicsWorld> dynamicsWorld;
	GameMachine* gameMachine;
	std::vector<GameObject*> shapes;
	std::vector<GameLight*> lights;
	Character* character;
	GameObject* sky;
	GMfloat ellapsed;
	UpAxis upAxis;
};

END_NS
#endif