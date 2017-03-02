#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include "common.h"
#include "gameworldprivate.h"

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
	virtual ~GameWorld();

public:
	virtual void renderGameWorld() = 0;

public:
	void initialize();
	void appendObjectAndInit(AUTORELEASE GameObject* obj);
	void appendLight(AUTORELEASE GameLight* light);
	std::vector<GameLight*>& getLights();
	void setMajorCharacter(Character* character);
	Character* getMajorCharacter();
	void simulateGameWorld(GMfloat elapsed);
	PhysicsWorld* physicsWorld();
	IGraphicEngine* getGraphicEngine();
	void setGameMachine(GameMachine* gm);
	GameMachine* getGameMachine();
	GMfloat getElapsed();

private:
	void createPainterForObject(GameObject* obj);
};

END_NS
#endif