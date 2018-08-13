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
		GMModels* models = object->getModels();
		if (models)
		{
			for (auto& model : models->getModels())
			{
				if (model.getModel()->getShader().getBlend())
					return true;
			}
		}
		else
		{
			GMModel* model = object->getModel();
			if (model && model->getShader().getBlend())
				return true;
		}
		return false;
	}
}

GMGameWorld::GMGameWorld(const IRenderContext* context)
{
	D(d);
	d->context = context;
}

void GMGameWorld::addObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->setContext(getContext());
	obj->onAppendingObjectToWorld();
	d->gameObjects.insert(GMOwnedPtr<GMGameObject>(obj));

	obj->foreach([d](GMModel* m) {
		GM.createModelDataProxyAndTransfer(d->context, m);
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
}

bool GMGameWorld::removeObject(GMGameObject* obj)
{
	D(d);
	auto& objs = d->gameObjects;
	auto objIter = objs.find(GMOwnedPtr<GMGameObject>(obj));
	if (objIter == objs.end())
		return false;
	GMGameObject* eraseTarget = (*objIter).get();
	obj->onRemovingObjectFromWorld();
	objs.erase(objIter);
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
}

void GMGameWorld::clearRenderList()
{
	D(d);
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

void GMGameWorld::setPhysicsWorld(AUTORELEASE GMPhysicsWorld* w)
{
	D(d);
	GM_ASSERT(!d->physicsWorld);
	d->physicsWorld.reset(w);
}