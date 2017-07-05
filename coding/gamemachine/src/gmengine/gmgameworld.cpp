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
	memset(&d->graphicEnv, 0, sizeof(d->graphicEnv));
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
	engine->setEnvironment(d->graphicEnv);
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

void GMGameWorld::setDefaultAmbientLight(const GMLight& lightInfo)
{
	D(d);
	d->graphicEnv.ambientLightColor[0] = lightInfo.getLightColor()[0];
	d->graphicEnv.ambientLightColor[1] = lightInfo.getLightColor()[1];
	d->graphicEnv.ambientLightColor[2] = lightInfo.getLightColor()[2];
	d->graphicEnv.ambientK[0] = lightInfo.getKa()[0];
	d->graphicEnv.ambientK[1] = lightInfo.getKa()[1];
	d->graphicEnv.ambientK[2] = lightInfo.getKa()[2];
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