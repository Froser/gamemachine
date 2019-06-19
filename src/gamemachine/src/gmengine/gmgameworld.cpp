#include "stdafx.h"
#include "gmgameworld.h"
#include "gameobjects/gmgameobject.h"
#include "gmdata/gmmodel.h"
#include <algorithm>
#include <time.h>
#include "foundation/gamemachine.h"

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

GMGameWorld::GMGameWorld(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->particleSystem.reset(new GMParticleSystemManager(context));
}

void GMGameWorld::addObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	if (obj->getWorld() == this)
		return; //inited

	GMOwnedPtr<GMMutex, GMMutexRelease> mutexGuard(&d->addObjectMutex);
	mutexGuard->lock();

	obj->setWorld(this);
	obj->setContext(getContext());
	obj->onAppendingObjectToWorld();
	d->gameObjects.insert(GMOwnedPtr<GMGameObject>(obj));

	obj->foreachModel([d, this](GMModel* m) {
		getContext()->getEngine()->createModelDataProxy(d->context, m);
	});
}

void GMGameWorld::renderScene()
{
	D(d);
	static List<GMGameObject*> s_emptyList;
	IGraphicEngine* engine = d->context->getEngine();
	if (getRenderPreference() == GMRenderPreference::PreferForwardRendering)
	{
		engine->draw(d->renderList.deferred, s_emptyList);
		engine->draw(d->renderList.forward, s_emptyList);
	}
	else
	{
		engine->draw(d->renderList.forward, d->renderList.deferred);
	}

	d->particleSystem->render();
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

void GMGameWorld::updateGameWorld(GMDuration dt)
{
	D(d);
	auto phyw = getPhysicsWorld();
	updateGameObjects(dt, phyw, d->gameObjects);
	d->particleSystem->update(dt);
}

void GMGameWorld::clearRenderList()
{
	D(d);
	GMOwnedPtr<GMMutex, GMMutexRelease> mutexGuard(&d->renderListMutex);
	mutexGuard->lock();
	d->renderList.deferred.clear();
	d->renderList.forward.clear();
}

void GMGameWorld::updateGameObjects(GMDuration dt, GMPhysicsWorld* phyw, const Set<GMOwnedPtr<GMGameObject>>& gameObjects)
{
	for (decltype(auto) gameObject : gameObjects)
	{
		gameObject->update(dt);
		if (phyw)
			phyw->update(dt, gameObject.get());
	}
}

void GMGameWorld::addToRenderList(GMGameObject* object)
{
	D(d);
	GMOwnedPtr<GMMutex, GMMutexRelease> mutexGuard(&d->renderListMutex);
	mutexGuard->lock();

	if (object->canDeferredRendering())
	{
		d->renderList.deferred.push_back(object);
	}
	else
	{
		if (needBlend(object))
			d->renderList.forward.push_back(object);
		else
			d->renderList.forward.push_front(object);
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

void GMGameWorld::setPhysicsWorld(AUTORELEASE GMPhysicsWorld* w)
{
	D(d);
	GM_ASSERT(!d->physicsWorld);
	d->physicsWorld.reset(w);
}