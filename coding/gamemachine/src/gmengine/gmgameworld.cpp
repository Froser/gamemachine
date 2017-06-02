#include "stdafx.h"
#include "gmgameworld.h"
#include "gmgameobject.h"
#include "gmdatacore/object.h"
#include "gmcharacter.h"
#include "gmengine/controllers/graphic_engine.h"
#include <algorithm>
#include <time.h>
#include "foundation/gamemachine.h"

GMGameWorld::GMGameWorld(GamePackage* pk)
{
	D(d);
	d->gamePackage = pk;
	d->start = false;
}

GMGameWorld::~GMGameWorld()
{
	D(d);
	for (auto iter = d->shapes.begin(); iter != d->shapes.end(); iter++)
	{
		delete *iter;
	}
}

void GMGameWorld::initialize()
{
	GameMachine::instance().getGraphicEngine()->setCurrentWorld(this);
}

void GMGameWorld::appendObjectAndInit(AUTORELEASE GMGameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->onAppendingObjectToWorld();
	d->shapes.insert(obj);
	GameMachine::instance().initObjectPainter(obj);
}

void GMGameWorld::setMajorCharacter(GMCharacter* character)
{
	D(d);
	d->character = character;
}

GMCharacter* GMGameWorld::getMajorCharacter()
{
	D(d);
	return d->character;
}

void GMGameWorld::simulateGameWorld()
{
	D(d);
	physicsWorld()->simulate();
	d->character->simulation();
	if (!d->start) // 第一次simulate
		d->start = true;
}

GamePackage* GMGameWorld::getGamePackage()
{
	D(d);
	return d->gamePackage;
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

void GMGameWorld::setDefaultAmbientLight(const LightInfo& lightInfo)
{
	D(d);
	d->ambientLight = lightInfo;
}

LightInfo& GMGameWorld::getDefaultAmbientLight()
{
	D(d);
	return d->ambientLight;
}