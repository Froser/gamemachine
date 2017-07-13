#include "stdafx.h"
#include "gmgameworld.h"
#include "gmgameobject.h"
#include "gmdatacore/object.h"
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
	GameMachine::instance().initObjectPainter(obj);
}

void GMGameWorld::simulateGameWorld()
{
	D(d);
	// 仅仅对Entity和Sprite进行simulate
	auto phyw = physicsWorld();
	for (auto& gameObject : d->gameObjects[GMGameObjectType::Entity])
	{
		gameObject->simulate();
		phyw->simulate(gameObject);
		gameObject->updateAfterSimulate();
	}

	for (auto& gameObject : d->gameObjects[GMGameObjectType::Sprite])
	{
		gameObject->simulate();
		phyw->simulate(gameObject);
		gameObject->updateAfterSimulate();
	}

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
	factory->createPainter(engine, obj->getObject(), &painter);
	ASSERT(!obj->getObject()->getPainter());
	obj->getObject()->setPainter(painter);
	return painter;
}