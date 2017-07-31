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

void GMGameWorld::renderGameWorld()
{
	D(d);
	IGraphicEngine* engine = GameMachine::instance().getGraphicEngine();
	engine->newFrame();
}

void GMGameWorld::appendObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->onAppendingObjectToWorld();
	d->gameObjects[obj->getType()].insert(obj);
	GameMachine::instance().initObjectPainter(obj->getModel());
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

GMObjectPainter* GMGameWorld::createPainterForObject(GMGameObject* obj)
{
	D(d);
	GameMachine& gm = GameMachine::instance();
	IFactory* factory = gm.getFactory();
	IGraphicEngine* engine = gm.getGraphicEngine();
	GMObjectPainter* painter;
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