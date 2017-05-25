#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include "common.h"
#include "gameworldprivate.h"

BEGIN_NS
class Character;
class GameMachine;
struct IGraphicEngine;
class GameObject;
class ObjectPainter;
class GameWorld
{
	DECLARE_PRIVATE(GameWorld)
	GM_DECLARE_ALIGNED_ALLOCATOR();

public:
	GameWorld(GamePackage* pk);
	virtual ~GameWorld();

public:
	virtual void renderGameWorld() = 0;
	virtual PhysicsWorld* physicsWorld() = 0;

public:
	void initialize();
	void appendObjectAndInit(AUTORELEASE GameObject* obj);
	void initObject(GameObject* obj);
	void simulateGameWorld();
	IGraphicEngine* getGraphicEngine();
	void setGameMachine(GameMachine* gm);
	GameMachine* getGameMachine();
	GamePackage* getGamePackage();
	GMfloat getElapsed();
	void setDefaultAmbientLight(const LightInfo& lightInfo);
	LightInfo& getDefaultAmbientLight();

// characters:
public:
	virtual void setMajorCharacter(Character* character);
	Character* getMajorCharacter();

private:
	ObjectPainter* createPainterForObject(GameObject* obj);
};

END_NS
#endif