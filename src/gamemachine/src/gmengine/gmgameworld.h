#ifndef __GAMEWORLD_H__
#define __GAMEWORLD_H__
#include <gmcommon.h>
#include <gamemachine.h>
#include "../gmphysics/gmphysicsworld.h"
#include <gmenums.h>
#include "gameobjects/gmgameobject.h"
#include <gmassets.h>
#include <gmparticle.h>

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
	GMGameObjectContainer forward;
	GMGameObjectContainer deferred;
};

GM_PRIVATE_CLASS(GMGameWorld);
class GM_EXPORT GMGameWorld : public GMObject
{
	GM_DECLARE_PRIVATE(GMGameWorld)
	GM_FRIEND_CLASS(GMPhysicsWorld)
	GM_DECLARE_PROPERTY(GMRenderPreference, RenderPreference)

public:
	GMGameWorld(const IRenderContext* context);
	~GMGameWorld();

public:
	GMPhysicsWorld* getPhysicsWorld();
	void setPhysicsWorld(AUTORELEASE GMPhysicsWorld* w);

public:
	virtual void renderScene();
	virtual bool removeObject(GMGameObject* obj);
	virtual const IRenderContext* getContext();
	virtual void setParticleSystemManager(AUTORELEASE IParticleSystemManager* particleSystemManager);

public:
	void addObjectAndInit(AUTORELEASE GMGameObject* obj);
	void updateGameWorld(GMDuration dt);
	void clearRenderList();
	void addToRenderList(GMGameObject* object);
	bool removeFromRenderList(GMGameObject* object);
	IParticleSystemManager* getParticleSystemManager();
	GMAssets& getAssets();

protected:
	GMRenderList& getRenderList();
};

END_NS
#endif