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
class GMModelDataProxy;
class GMPhysicsWorld;

enum class GMRenderPreference
{
	PreferForwardRendering,
	PreferDeferredRendering,
};

struct GMRenderList
{
	List<GMGameObject*> forward;
	List<GMGameObject*> deferred;
};

GM_PRIVATE_OBJECT(GMGameWorld)
{
	const IRenderContext* context = nullptr;
	GMPhysicsWorld* physicsWorld = nullptr;
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
	GMGameWorld(const IRenderContext* context);
	virtual ~GMGameWorld();

public:
	GMPhysicsWorld* getPhysicsWorld() { D(d); return d->physicsWorld; }

public:
	virtual void renderScene();
	virtual bool removeObject(GMGameObject* obj);
	virtual const IRenderContext* getContext();

public:
	void addObjectAndInit(AUTORELEASE GMGameObject* obj);
	void simulateGameWorld();
	void clearRenderList();
	void addToRenderList(GMGameObject* object);
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