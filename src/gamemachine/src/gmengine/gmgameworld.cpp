#include "stdafx.h"
#include "gmgameworld.h"
#include "gameobjects/gmgameobject.h"
#include "gmdata/gmmodel.h"
#include <algorithm>
#include <time.h>
#include "foundation/gamemachine.h"
#include "gameobjects/gmcontrolgameobject.h"

GMGameWorld::GMGameWorld()
{
	D(d);
	d->start = false;
}

GMGameWorld::~GMGameWorld()
{
	D(d);
	for (auto types : d->gameObjects)
	{
		for (auto gameObject : types.second)
		{
			delete gameObject;
		}
	}

	for (auto control : d->controls)
	{
		delete control;
	}
}

void GMGameWorld::addObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->onAppendingObjectToWorld();
	d->gameObjects[obj->getType()].insert(obj);
	GM.createModelPainter(obj->getModel());
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
	auto phyw = physicsWorld();
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
	GM.createModelPainter(control->getModel());
}

void GMGameWorld::notifyControls()
{
	D(d);
	for (auto& obj : d->controls)
	{
		obj->notifyControl();
	}
}

GMModelPainter* GMGameWorld::createPainterForObject(GMGameObject* obj)
{
	D(d);
	IFactory* factory = GM.getFactory();
	IGraphicEngine* engine = GM.getGraphicEngine();
	GMModelPainter* painter;
	factory->createPainter(engine, obj->getModel(), &painter);
	GM_ASSERT(!obj->getModel()->getPainter());
	obj->getModel()->setPainter(painter);
	return painter;
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