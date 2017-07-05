#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include "common.h"
#include "foundation/vector.h"
#include "gmphysics/gmphysicsworld.h"
#include "gmdatacore/shader.h"
#include "gmgameobject.h"

BEGIN_NS

class GMCharacter;
class ObjectPainter;

GM_PRIVATE_OBJECT(GMGameWorld)
{
	Map<GMGameObjectType, Set<GMGameObject*> > gameObjects;
	GMGraphicEnvironment graphicEnv;
	bool start;
};

class GMGameWorld : public GMObject
{
	DECLARE_PRIVATE(GMGameWorld)

public:
	GMGameWorld();
	virtual ~GMGameWorld();

public:
	virtual GMPhysicsWorld* physicsWorld() = 0;

public:
	virtual void renderGameWorld();

public:
	void appendObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	void setDefaultAmbientLight(const GMLightInfo& lightInfo);
	Set<GMGameObject*>& getGameObjects(GMGameObjectType type) { D(d); return d->gameObjects[type]; }

private:
	ObjectPainter* createPainterForObject(GMGameObject* obj);
};

END_NS
#endif