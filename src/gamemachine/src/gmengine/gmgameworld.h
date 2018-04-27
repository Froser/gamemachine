#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include "../gmphysics/gmphysicsworld.h"
#include <gmenums.h>
#include "gameobjects/gmgameobject.h"
#include <gmassets.h>

BEGIN_NS

class GMControlGameObject;
class GMCharacter;
class GMModelPainter;
class GMPhysicsWorld;

enum class GMRenderPreference
{
	PreferForwardRendering,
	PreferDeferredRendering,
};

GM_PRIVATE_OBJECT(GMGameWorld)
{
	GMPhysicsWorld* physicsWorld = nullptr;
	Map<GMGameObjectType, Set<GMGameObject*> > gameObjects;
	Vector<GMControlGameObject*> controls;
	Vector<GMGameObject*> controls_objectType;
	GMAssets assets;
	bool start = false;
	GMRenderPreference renderPreference = GMRenderPreference::PreferForwardRendering;
};

class GMGameWorld : public GMObject
{
	DECLARE_PRIVATE(GMGameWorld)
	GM_FRIEND_CLASS(GMPhysicsWorld)
	GM_DECLARE_PROPERTY(RenderPreference, renderPreference, GMRenderPreference)

public:
	GMGameWorld() = default;
	virtual ~GMGameWorld();

public:
	GMPhysicsWorld* getPhysicsWorld() { D(d); return d->physicsWorld; }

public:
	virtual void renderScene() {}
	virtual bool removeObject(GMGameObject* obj);

public:
	void addObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	void addControl(AUTORELEASE GMControlGameObject* control);
	void notifyControls();

	inline Vector<GMControlGameObject*>& getControls() { D(d); return d->controls; }
	inline Vector<GMGameObject*>& getControlsGameObject() { D(d); return d->controls_objectType; }
	inline GMAssets& getAssets() { D(d); return d->assets; }

	const Set<GMGameObject*>& getGameObjects(GMGameObjectType type);
	void addLight(const GMLight& light);
	void removeLights();

private:
	void simulateGameObjects(GMPhysicsWorld* phyw, Set<GMGameObject*> gameObjects);

	// GMPhysicsWorld
private:
	void setPhysicsWorld(AUTORELEASE GMPhysicsWorld* w) { D(d); GM_ASSERT(!d->physicsWorld); d->physicsWorld = w; }
};

END_NS
#endif