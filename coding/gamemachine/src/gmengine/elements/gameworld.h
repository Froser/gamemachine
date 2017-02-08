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
class Character;
class GameMachine;
struct IGraphicEngine;
class GameLight;
class GameObject;

class GameWorld
{
	DEFINE_PRIVATE(GameWorld)
public:
	GameWorld();
	~GameWorld();

public:
	void initialize();
	void appendObject(AUTORELEASE GameObject* obj);
	void appendLight(AUTORELEASE GameLight* light);
	std::vector<GameLight*>& getLights();
	void setMajorCharacter(Character* character);
	Character* getMajorCharacter();
	void setSky(GameObject* sky);
	GameObject* getSky();
	Script* getScript();
	void simulateGameWorld(GMfloat elapsed);
	void renderGameWorld();
	void setGravity(GMfloat x, GMfloat y, GMfloat z);

	IGraphicEngine* getGraphicEngine();
	void setGameMachine(GameMachine* gm);
	GameMachine* getGameMachine();
	GMfloat getElapsed();
	
	GameObject* findGameObjectById(GMuint id);

	void dispatchEvents();

private:
	void createPainterForObject(GameObject* obj);
};

END_NS
#endif