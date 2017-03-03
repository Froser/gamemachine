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
	virtual PhysicsWorld* physicsWorld() = 0;

public:
	void initialize();
	void appendObjectAndInit(AUTORELEASE GameObject* obj);
	void appendLight(AUTORELEASE GameLight* light);
	std::vector<GameLight*>& getLights();
	void simulateGameWorld(GMfloat elapsed);
	IGraphicEngine* getGraphicEngine();
	void setGameMachine(GameMachine* gm);
	GameMachine* getGameMachine();
	GMfloat getElapsed();

	// characters:
public:
	virtual void setMajorCharacter(Character* character);
	Character* getMajorCharacter();

private:
	void createPainterForObject(GameObject* obj);
};

END_NS
#endif