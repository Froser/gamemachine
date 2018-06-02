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
		GMModels& models = object->getModels();
		for (auto& model : models)
		{
			if (model->getShader().getBlend())
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

GMGameWorld::~GMGameWorld()
{
	D(d);
	for (auto gameObject : d->gameObjects)
	{
		GM_delete(gameObject);
	}

	GM_delete(d->physicsWorld);
}

void GMGameWorld::addObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->setContext(getContext());
	obj->onAppendingObjectToWorld();
	d->gameObjects.insert(obj);
	GMModels& models = obj->getModels();
	for (auto& model : models)
	{
		GM.createModelDataProxyAndTransfer(d->context, model);
	}
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
	auto objIter = objs.find(obj);
	if (objIter == objs.end())
		return false;
	GMGameObject* eraseTarget = *objIter;
	obj->onRemovingObjectFromWorld();
	objs.erase(objIter);
	delete eraseTarget;
	return true;
}

const IRenderContext* GMGameWorld::getContext()
{
	D(d);
	return d->context;
}

void GMGameWorld::simulateGameWorld()
{
	D(d);
	auto phyw = getPhysicsWorld();
	simulateGameObjects(phyw, d->gameObjects);
}

void GMGameWorld::clearRenderList()
{
	D(d);
	d->renderList.deferred.clear();
	d->renderList.forward.clear();
}

void GMGameWorld::simulateGameObjects(GMPhysicsWorld* phyw, const Set<GMGameObject*>& gameObjects)
{
	for (auto& gameObject : gameObjects)
	{
		gameObject->simulate();
		if (phyw)
			phyw->simulate(gameObject);
		gameObject->updateAfterSimulate();
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