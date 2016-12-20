#ifndef __GAMESCENE_H__
#define __GAMESCENE_H__
#include "common.h"
#include "gameworldprivate.h"

class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;

BEGIN_NS
class Object;
class Character;
class GameWorld
{
	DEFINE_PRIVATE(GameWorld)
public:
	GameWorld();

public:
	void appendObject(GameObject* obj);
	void setMajorCharacter(Character* character);
	Character* getMajorCharacter();
	void simulateGameWorld(GMint fps);
	void renderGameWorld();
	void setGravity(GMfloat x, GMfloat y, GMfloat z);
};

END_NS
#endif