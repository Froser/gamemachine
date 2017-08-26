#include "stdafx.h"
#include "gmgameworld.h"
#include "gmgameobject.h"
#include "gmdatacore/gmmodel.h"
#include <algorithm>
#include <time.h>
#include "foundation/gamemachine.h"

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
}

void GMGameWorld::appendObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->onAppendingObjectToWorld();
	d->gameObjects[obj->getType()].insert(obj);
	GM.initObjectPainter(obj->getModel());
}

void GMGameWorld::beginCreateStencil()
{
	IGraphicEngine* engine = GM. getGraphicEngine();
	engine->beginCreateStencil();
}

void GMGameWorld::endCreateStencil()
{
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->endCreateStencil();
}

void GMGameWorld::beginUseStencil(bool inverse)
{
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->beginUseStencil(inverse);
}

void GMGameWorld::endUseStencil()
{
	IGraphicEngine* engine = GM.getGraphicEngine();
	engine->endUseStencil();
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

GMModelPainter* GMGameWorld::createPainterForObject(GMGameObject* obj)
{
	D(d);
	IFactory* factory = GM.getFactory();
	IGraphicEngine* engine = GM.getGraphicEngine();
	GMModelPainter* painter;
	factory->createPainter(engine, obj->getModel(), &painter);
	ASSERT(!obj->getModel()->getPainter());
	obj->getModel()->setPainter(painter);
	return painter;
}

void GMGameWorld::simulateGameObjects(GMPhysicsWorld* phyw, Set<GMGameObject*> gameObjects)
{
	std::for_each(gameObjects.cbegin(), gameObjects.cend(), [phyw](GMGameObject* gameObject) {
		gameObject->simulate();
		if (phyw)
			phyw->simulate(gameObject);
		gameObject->updateAfterSimulate();
	});
}