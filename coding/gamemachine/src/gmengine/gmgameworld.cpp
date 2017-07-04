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
	for (auto& gameObject : d->gameObjects[GMGameObjectType::Entity])
	{
		gameObject->simulate();
		physicsWorld()->simulate(gameObject);
	}

	for (auto& gameObject : d->gameObjects[GMGameObjectType::Sprite])
	{
		gameObject->simulate();
		physicsWorld()->simulate(gameObject);
	}

	if (!d->start) // 第一次simulate
		d->start = true;
}

ObjectPainter* GMGameWorld::createPainterForObject(GMGameObject* obj)
{
	D(d);
	GameMachine& gm = GameMachine::instance();
	IFactory* factory = gm.getFactory();
	IGraphicEngine* engine = gm.getGraphicEngine();
	ObjectPainter* painter;
	factory->createPainter(engine, obj->getObject(), &painter);
	ASSERT(!obj->getObject()->getPainter());
	obj->getObject()->setPainter(painter);
	return painter;
}