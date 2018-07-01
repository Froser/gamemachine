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
	GMOwnedPtr<GMPhysicsWorld> physicsWorld = nullptr;
	Set<GMOwnedPtr<GMGameObject>> gameObjects;
	GMAssets assets;
	GMRenderPreference renderPreference = GMRenderPreference::PreferForwardRendering;
	GMRenderList renderList;
};

class GMGameWorld : public GMObject
{
	GM_DECLARE_PRIVATE(GMGameWorld)
	GM_FRIEND_CLASS(GMPhysicsWorld)
	GM_DECLARE_PROPERTY(RenderPreference, renderPreference, GMRenderPreference)

public:
	GMGameWorld(const IRenderContext* context);
	~GMGameWorld() = default;

public:
	GMPhysicsWorld* getPhysicsWorld() { D(d); return d->physicsWorld.get(); }

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
	void simulateGameObjects(GMPhysicsWorld* phyw, const Set<GMOwnedPtr<GMGameObject>>& gameObjects);

	// GMPhysicsWorld
private:
	void setPhysicsWorld(AUTORELEASE GMPhysicsWorld* w);
};

END_NS
#endif