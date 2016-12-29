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
struct IGraphicEngine;

class GameWorld
{
	DEFINE_PRIVATE(GameWorld)
public:
	GameWorld();

public:
	void initialize();

public:
	void appendObject(AUTORELEASE GameObject* obj);
	void setMajorCharacter(Character* character);
	Character* getMajorCharacter();
	void setSky(GameObject* sky);
	GameObject* getSky();
	void simulateGameWorld(GMint fps);
	void renderGameWorld();
	void setGravity(GMfloat x, GMfloat y, GMfloat z);

	IGraphicEngine* getGraphicEngine();
};

END_NS
#endif