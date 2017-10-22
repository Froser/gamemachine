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
class GMCharacter;
class GMModelPainter;
GM_PRIVATE_OBJECT(GMGameWorld)
{
	Map<GMGameObjectType, Set<GMGameObject*> > gameObjects;
	Vector<GMControlGameObject*> controls;
	Vector<GMGameObject*> controls_objectType;
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
	virtual GMPhysicsWorld* physicsWorld() { return nullptr; }

public:
	virtual void renderScene() {}
	virtual bool removeObject(GMGameObject* obj);

public:
	void appendObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	void addControl(AUTORELEASE GMControlGameObject* control);
	void notifyControls();

	inline Vector<GMControlGameObject*>& getControls() { D(d); return d->controls; }
	inline Vector<GMGameObject*>& getControlsGameObject() { D(d); return d->controls_objectType; }
	inline const Set<GMGameObject*>& getGameObjects(GMGameObjectType type) { D(d); return d->gameObjects[type]; }
	inline void addLight(const GMLight& light) { GM.getGraphicEngine()->addLight(light); }
	inline void removeLights() { GM.getGraphicEngine()->removeLights(); }
	inline GMAssets& getAssets() { D(d); return d->assets; }

private:
	GMModelPainter* createPainterForObject(GMGameObject* obj);
	void simulateGameObjects(GMPhysicsWorld* phyw, Set<GMGameObject*> gameObjects);
};

END_NS
#endif