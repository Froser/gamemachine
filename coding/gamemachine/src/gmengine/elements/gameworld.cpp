#include "stdafx.h"
#include "gameworld.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
#include "utilities/assert.h"
#include "character.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmengine/controllers/gamemachine.h"
#include <algorithm>
#include "gmengine/controllers/factory.h"
#include <time.h>

GameWorld::GameWorld(GamePackage* pk)
{
	D(d);
	d.gamePackage = pk;
	d.elapsed = 0;
	d.start = false;
}

GameWorld::~GameWorld()
{
	D(d);
	for (auto iter = d.shapes.begin(); iter != d.shapes.end(); iter++)
	{
		delete *iter;
	}
}

void GameWorld::initialize()
{
	getGraphicEngine()->initialize(this);
}

void GameWorld::appendObjectAndInit(AUTORELEASE GameObject* obj)
{
	D(d);
	obj->setWorld(this);
	obj->onAppendingObjectToWorld();
	d.shapes.insert(obj);
	initObject(obj);
}

// initObject在于初始化GameObject中的Object，为其创建一个Painter，并且将数据传送到显卡
void GameWorld::initObject(GameObject* obj)
{
	// 创建一个Painter
	ObjectPainter* painter = createPainterForObject(obj);
	ASSERT(painter);
	painter->transfer();
}

void GameWorld::setMajorCharacter(Character* character)
{
	D(d);
	d.character = character;
}

Character* GameWorld::getMajorCharacter()
{
	D(d);
	return d.character;
}

void GameWorld::simulateGameWorld()
{
	D(d);
	physicsWorld()->simulate();
	d.character->simulation();
	if (!d.start) // 第一次simulate
	{
		d.startTick = clock();
		d.elapsed = 0;
		d.start = true;
	}
	else
	{
		d.elapsed = (GMfloat)(clock() - d.startTick) * .001f;
	}
}

IGraphicEngine* GameWorld::getGraphicEngine()
{
	D(d);
	return d.gameMachine->getGraphicEngine();
}

void GameWorld::setGameMachine(GameMachine* gm)
{
	D(d);
	d.gameMachine = gm;
}

GameMachine* GameWorld::getGameMachine()
{
	D(d);
	return d.gameMachine;
}

GamePackage* GameWorld::getGamePackage()
{
	D(d);
	return d.gamePackage;
}

GMfloat GameWorld::getElapsed()
{
	D(d);
	return d.elapsed;
}

ObjectPainter* GameWorld::createPainterForObject(GameObject* obj)
{
	D(d);
	GameMachine* gm = d.gameMachine;
	IFactory* factory = gm->getFactory();
	IGraphicEngine* engine = gm->getGraphicEngine();
	ObjectPainter* painter;
	factory->createPainter(engine, obj->getObject(), &painter);
	ASSERT(!obj->getObject()->getPainter());
	obj->getObject()->setPainter(painter);
	return painter;
}

void GameWorld::setDefaultAmbientLight(const LightInfo& lightInfo)
{
	D(d);
	d.ambientLight = lightInfo;
}

LightInfo& GameWorld::getDefaultAmbientLight()
{
	D(d);
	return d.ambientLight;
}