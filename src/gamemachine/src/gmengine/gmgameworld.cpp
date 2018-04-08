#include "stdafx.h"
#include "gmgameworld.h"
#include "gameobjects/gmgameobject.h"
#include "gmdata/gmmodel.h"
#include <algorithm>
#include <time.h>
#include "foundation/gamemachine.h"
#include "gameobjects/gmcontrolgameobject.h"

GMGameWorld::~GMGameWorld()
{
	D(d);
	for (auto types : d->gameObjects)
	{
		for (auto gameObject : types.second)
		{
			GM_delete(gameObject);
		}
	}

	for (auto control : d->controls)
	{
		GM_delete(control);
	}

	GM_delete(d->physicsWorld);
}

void GMGameWorld::addObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->onAppendingObjectToWorld();
	d->gameObjects[obj->getType()].insert(obj);
	GMModels& models = obj->getModels();
	for (auto& model : models)
	{
		GM.createModelPainterAndTransfer(model);
	}
}

bool GMGameWorld::removeObject(GMGameObject* obj)
{
	D(d);
	auto& objs = d->gameObjects[obj->getType()];
	auto objIter = objs.find(obj);
	if (objIter == objs.end())
		return false;
	GMGameObject* eraseTarget = *objIter;
	obj->onRemovingObjectFromWorld();
	objs.erase(objIter);
	delete eraseTarget;
	return true;
}

void GMGameWorld::simulateGameWorld()
{
	D(d);
	auto phyw = getPhysicsWorld();
	simulateGameObjects(phyw, d->gameObjects[GMGameObjectType::Entity]);
	simulateGameObjects(phyw, d->gameObjects[GMGameObjectType::Sprite]);
	simulateGameObjects(phyw, d->gameObjects[GMGameObjectType::Particles]);

	if (!d->start) // 第一次simulate
		d->start = true;
}

void GMGameWorld::addControl(GMControlGameObject* control)
{
	D(d);
	control->setWorld(this);
	control->onAppendingObjectToWorld();
	d->controls.push_back(control);
	d->controls_objectType.push_back(control);
}

void GMGameWorld::notifyControls()
{
	D(d);
	for (auto& obj : d->controls)
	{
		obj->notifyControl();
	}
}

void GMGameWorld::simulateGameObjects(GMPhysicsWorld* phyw, Set<GMGameObject*> gameObjects)
{
	for (auto& gameObject : gameObjects)
	{
		gameObject->simulate();
		if (phyw)
			phyw->simulate(gameObject);
		gameObject->updateAfterSimulate();
	}
}