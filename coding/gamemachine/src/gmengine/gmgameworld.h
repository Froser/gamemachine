#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include <gmcommon.h>
#include "../foundation/gamemachine.h"
#include "../gmphysics/gmphysicsworld.h"
#include <shader.h>
#include "gameobjects/gmgameobject.h"
#include <gmassets.h>

BEGIN_NS

class GMControlGameObject;
struct GMControlGameObjectPredicator
{
	bool operator()(const GMControlGameObject& lhs, const GMControlGameObject& rhs);
};

class GMCharacter;
class GMModelPainter;
GM_PRIVATE_OBJECT(GMGameWorld)
{
	Map<GMGameObjectType, Set<GMGameObject*> > gameObjects;
	Set<GMControlGameObject*> controls;
	GMAssets assets;
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
	void addControl(AUTORELEASE GMControlGameObject* control);
	void notifyControls();

	inline const Set<GMControlGameObject*>& getControls() { D(d); return d->controls; }
	inline const Set<GMGameObject*>& getGameObjects(GMGameObjectType type) { D(d); return d->gameObjects[type]; }
	inline void addLight(const GMLight& light) { GameMachine::instance().getGraphicEngine()->addLight(light); }
	inline GMAssets& getAssets() { D(d); return d->assets; }

private:
	GMModelPainter* createPainterForObject(GMGameObject* obj);
	void simulateGameObjects(GMPhysicsWorld* phyw, Set<GMGameObject*> gameObjects);
};

END_NS
#endif