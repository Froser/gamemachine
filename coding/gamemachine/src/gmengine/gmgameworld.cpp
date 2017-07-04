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
	for (auto& gameObject : d->gameObjects[GMGameObjectType::Entity])
	{
		gameObject->simulate();
		physicsWorld()->simulate(gameObject);
		gameObject->updateAfterSimulate();
	}

	for (auto& gameObject : d->gameObjects[GMGameObjectType::Sprite])
	{
		gameObject->simulate();
		physicsWorld()->simulate(gameObject);
		gameObject->updateAfterSimulate();
	}

	if (!d->start) // 第一次simulate
		d->start = true;
}

void GMGameWorld::setDefaultAmbientLight(const LightInfo& lightInfo)
{
	D(d);
	d->graphicEnv.ambientLightColor[0] = lightInfo.lightColor[0];
	d->graphicEnv.ambientLightColor[1] = lightInfo.lightColor[1];
	d->graphicEnv.ambientLightColor[2] = lightInfo.lightColor[2];
	d->graphicEnv.ambientK[0] = lightInfo.args[LA_KA];
	d->graphicEnv.ambientK[1] = lightInfo.args[LA_KA + 1];
	d->graphicEnv.ambientK[2] = lightInfo.args[LA_KA + 2];
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