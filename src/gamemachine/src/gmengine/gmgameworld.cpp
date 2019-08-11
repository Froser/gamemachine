#include "stdafx.h"
#include "gmgameworld.h"
#include "gameobjects/gmgameobject.h"
#include "gmdata/gmmodel.h"
#include <algorithm>
#include <time.h>
#include "foundation/gamemachine.h"
#include "gmphysics/gmphysicsworld_p.h"

BEGIN_NS

namespace
{
	bool needBlend(GMGameObject* object)
	{
		GMScene* scene = object->getScene();
		if (scene)
		{
			for (auto& model : scene->getModels())
			{
				if (model.getModel()->getShader().getBlend())
					return true;
			}
		}
		return false;
	}
}

GM_PRIVATE_OBJECT_UNALIGNED(GMGameWorld)
{
	const IRenderContext* context = nullptr;
	GMOwnedPtr<GMPhysicsWorld> physicsWorld = nullptr;
	Set<GMOwnedPtr<GMGameObject>> gameObjects;
	GMAssets assets;
	GMRenderPreference renderPreference = GMRenderPreference::PreferForwardRendering;
	GMRenderList renderList;
	GMMutex renderListMutex;
	GMMutex addObjectMutex;
	GMOwnedPtr<IParticleSystemManager> particleSystemMgr;
};

namespace
{
	void updateGameObjects(GMDuration dt, GMPhysicsWorld* phyw, const Set<GMOwnedPtr<GMGameObject>>& gameObjects)
	{
		for (decltype(auto) gameObject : gameObjects)
		{
			gameObject->update(dt);
			if (phyw)
				phyw->update(dt, gameObject.get());
		}
	}

	void addToContainerByPriority(GMGameObjectContainer& c, GMGameObject* o, GMGameObjectRenderPriority hint = GMGameObjectRenderPriority::Normal)
	{
		if (hint == GMGameObjectRenderPriority::High)
		{
			c.push_front(o);
		}
		else
		{
			GMGameObjectRenderPriority priority = o->getRenderPriority();
			if (priority == GMGameObjectRenderPriority::High)
				c.push_front(o);
			else
				c.push_back(o);
		}
	}
}

GM_DEFINE_PROPERTY(GMGameWorld, GMRenderPreference, RenderPreference, renderPreference)
GMGameWorld::GMGameWorld(const IRenderContext* context)
{
	GM_CREATE_DATA();

	D(d);
	d->context = context;
}

GMGameWorld::~GMGameWorld()
{

}

void GMGameWorld::addObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	if (obj->getWorld() == this)
		return; //inited

	GMMutexLock mutexGuard(&d->addObjectMutex);
	mutexGuard->lock();

	obj->setWorld(this);
	obj->setContext(getContext());
	obj->onAppendingObjectToWorld();
	d->gameObjects.insert(GMOwnedPtr<GMGameObject>(obj));

	obj->foreachModel([d, this](GMModel* m) {
		const IRenderContext* context = getContext();
		if (context->getEngine()->isCurrentMainThread())
			context->getEngine()->createModelDataProxy(d->context, m);
		else
			GM.invokeInMainThread([context, d, m]() {context->getEngine()->createModelDataProxy(d->context, m); });
	});
}

GMPhysicsWorld* GMGameWorld::getPhysicsWorld()
{
	D(d);
	return d->physicsWorld.get();
}

void GMGameWorld::renderScene()
{
	D(d);
	static GMGameObjectContainer s_emptyList;
	IGraphicEngine* engine = d->context->getEngine();
	engine->begin();
	if (d->particleSystemMgr)
		d->particleSystemMgr->render();

	if (getRenderPreference() == GMRenderPreference::PreferForwardRendering)
	{
		engine->draw(d->renderList.deferred, s_emptyList);
		engine->draw(d->renderList.forward, s_emptyList);
	}
	else
	{
		engine->draw(d->renderList.forward, d->renderList.deferred);
	}
	engine->end();
}

bool GMGameWorld::removeObject(GMGameObject* obj)
{
	D(d);
	auto& objs = d->gameObjects;
	const GMOwnedPtr<GMGameObject>* eraseTarget = nullptr;
	for (auto& o : objs)
	{
		if (o.get() == obj)
		{
			eraseTarget = &o;
			break;
		}
	}

	if (!eraseTarget)
		return false;

	removeFromRenderList(obj);
	obj->onRemovingObjectFromWorld();
	objs.erase(*eraseTarget);
	return true;
}

const IRenderContext* GMGameWorld::getContext()
{
	D(d);
	return d->context;
}

void GMGameWorld::setParticleSystemManager(AUTORELEASE IParticleSystemManager* particleSystemMgrManager)
{
	D(d);
	d->particleSystemMgr.reset(particleSystemMgrManager);
}

void GMGameWorld::updateGameWorld(GMDuration dt)
{
	D(d);
	auto phyw = getPhysicsWorld();
	updateGameObjects(dt, phyw, d->gameObjects);
	if (d->particleSystemMgr)
		d->particleSystemMgr->update(dt);
}

void GMGameWorld::clearRenderList()
{
	D(d);
	GMMutexLock mutexGuard(&d->renderListMutex);
	mutexGuard->lock();
	d->renderList.deferred.clear();
	d->renderList.forward.clear();
}

GMRenderList& GMGameWorld::getRenderList()
{
	D(d);
	return d->renderList;
}

void GMGameWorld::addToRenderList(GMGameObject* object)
{
	D(d);
	GMMutexLock mutexGuard(&d->renderListMutex);
	mutexGuard->lock();

	if (GMQueryCapability(GMCapability::SupportDeferredRendering) && object->canDeferredRendering())
	{
		addToContainerByPriority(d->renderList.deferred, object);
	}
	else
	{
		if (needBlend(object))
			addToContainerByPriority(d->renderList.forward, object);
		else
			addToContainerByPriority(d->renderList.forward, object, GMGameObjectRenderPriority::High);
	}
}

bool GMGameWorld::removeFromRenderList(GMGameObject* object)
{
	D(d);
	bool flag = false;
	{
		auto iter = std::find(d->renderList.forward.begin(), d->renderList.forward.end(), object);
		if (iter != d->renderList.forward.end())
		{
			d->renderList.forward.erase(iter);
			flag = true;
		}
	}

	{
		auto iter = std::find(d->renderList.deferred.begin(), d->renderList.deferred.end(), object);
		if (iter != d->renderList.deferred.end())
		{
			d->renderList.deferred.erase(iter);
			flag = true;
		}
	}

	return flag;
}

GMAssets& GMGameWorld::getAssets()
{
	D(d); return d->assets;
}

IParticleSystemManager* GMGameWorld::getParticleSystemManager()
{
	D(d); return d->particleSystemMgr.get();
}

void GMGameWorld::setPhysicsWorld(AUTORELEASE GMPhysicsWorld* w)
{
	{
		D(d);
		d->physicsWorld.reset(w);
	}

	if (w)
	{
		D_OF(d, w);
		if (d->world != this)
			d->world = this;
	}
}

END_NS
