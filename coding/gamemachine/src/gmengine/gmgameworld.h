#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include <gmcommon.h>
#include "../foundation/vector.h"
#include "../foundation/gamemachine.h"
#include "../gmphysics/gmphysicsworld.h"
#include "../gmdatacore/shader.h"
#include "gmgameobject.h"
#include "gmresourcecontainer.h"

BEGIN_NS

class GMCharacter;
class GMModelPainter;

GM_PRIVATE_OBJECT(GMGameWorld)
{
	Map<GMGameObjectType, Set<GMGameObject*> > gameObjects;
	GMResourceContainer resourceContainer;
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
	virtual void renderGameWorld() {}
	virtual void beginCreateStencil();
	virtual void endCreateStencil();
	virtual void beginUseStencil(bool inverse = false);
	virtual void endUseStencil();
	virtual bool removeObject(GMGameObject* obj);

public:
	void appendObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	Set<GMGameObject*>& getGameObjects(GMGameObjectType type) { D(d); return d->gameObjects[type]; }
	void addLight(const GMLight& light) { GameMachine::instance().getGraphicEngine()->addLight(light); }
	GMResourceContainer& getResourceContainer() { D(d); return d->resourceContainer; }

private:
	GMModelPainter* createPainterForObject(GMGameObject* obj);
	void simulateGameObjects(GMPhysicsWorld* phyw, Set<GMGameObject*> gameObjects);
};

END_NS
#endif