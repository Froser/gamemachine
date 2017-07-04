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
	LightInfo ambientLight;
	bool start;
};

class GMGameWorld : public GMObject
{
	DECLARE_PRIVATE(GMGameWorld)

public:
	GMGameWorld();
	virtual ~GMGameWorld();

public:
	virtual void renderGameWorld() = 0;
	virtual GMPhysicsWorld* physicsWorld() = 0;

public:
	void appendObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	void setDefaultAmbientLight(const LightInfo& lightInfo) { D(d); d->ambientLight = lightInfo; }
	LightInfo& getDefaultAmbientLight() { D(d); return d->ambientLight; }
	Set<GMGameObject*>& getGameObjects(GMGameObjectType type) { D(d); return d->gameObjects[type]; }

private:
	ObjectPainter* createPainterForObject(GMGameObject* obj);
};

END_NS
#endif