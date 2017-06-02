#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include "common.h"
#include "gameworldprivate.h"
#include "foundation/vector.h"

BEGIN_NS

class Character;
class ObjectPainter;
class GameWorld : public GMObject
{
	DECLARE_PRIVATE(GameWorld)

public:
	GameWorld(GamePackage* pk);
	virtual ~GameWorld();

public:
	virtual void renderGameWorld() = 0;
	virtual PhysicsWorld* physicsWorld() = 0;

public:
	void initialize();
	void appendObjectAndInit(AUTORELEASE GameObject* obj);
	void simulateGameWorld();
	GamePackage* getGamePackage();
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