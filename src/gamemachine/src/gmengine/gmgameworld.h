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

struct GMRenderList
{
	Vector<GMGameObject*> forward;
	Vector<GMGameObject*> deferred;
};

GM_PRIVATE_OBJECT(GMGameWorld)
{
	GMPhysicsWorld* physicsWorld = nullptr;
	Vector<GMControlGameObject*> controls;
	Vector<GMGameObject*> controls_objectType;
	GMAssets assets;
	GMRenderPreference renderPreference = GMRenderPreference::PreferForwardRendering;
	GMRenderList renderList;
	Set<GMGameObject*> gameObjects;
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
	virtual void renderScene();
	virtual bool removeObject(GMGameObject* obj);

public:
	void addObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	void addControl(AUTORELEASE GMControlGameObject* control);
	void notifyControls();
	void clearRenderList();
	void addToRenderList(GMGameObject* object);

	inline Vector<GMControlGameObject*>& getControls() { D(d); return d->controls; }
	inline Vector<GMGameObject*>& getControlsGameObject() { D(d); return d->controls_objectType; }
	inline GMAssets& getAssets() { D(d); return d->assets; }

protected:
	inline GMRenderList& getRenderList()
	{
		D(d);
		return d->renderList;
	}

private:
	void simulateGameObjects(GMPhysicsWorld* phyw, const Set<GMGameObject*>& gameObjects);

	// GMPhysicsWorld
private:
	void setPhysicsWorld(AUTORELEASE GMPhysicsWorld* w) { D(d); GM_ASSERT(!d->physicsWorld); d->physicsWorld = w; }
};

END_NS
#endif