#include "stdafx.h"
#include "gameworld.h"
#include "gameobject.h"
#include "gmdatacore/object.h"
#include "utilities/assert.h"
#include "character.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmengine/elements/gamelight.h"
#include "gmengine/controllers/gamemachine.h"
#include <algorithm>
#include "gmengine/controllers/factory.h"

GameWorld::GameWorld()
{
	D(d);
	d.physics.reset(new PhysicsWorld());
}

GameWorld::~GameWorld()
{
	D(d);
	for (auto iter = d.shapes.begin(); iter != d.shapes.end(); iter++)
	{
		delete *iter;
	}

	for (auto iter = d.lights.begin(); iter != d.lights.end(); iter++)
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
	if (std::find(d.shapes.begin(), d.shapes.end(), obj) != d.shapes.end())
		return;

	d.physics->addToPhysics(obj);
	obj->setWorld(this);
	obj->onAppendingObjectToWorld();

	d.shapes.push_back(obj);

	// 创建一个Painter
	createPainterForObject(obj);
	ObjectPainter* painter = obj->getObject()->getPainter();
	if (painter)
		painter->transfer();
}

void GameWorld::appendLight(AUTORELEASE GameLight* light)
{
	D(d);
	if (std::find(d.lights.begin(), d.lights.end(), light) != d.lights.end())
		return;

	d.lights.push_back(light);
	light->setWorld(this);
}

std::vector<GameLight*>& GameWorld::getLights()
{
	D(d);
	return d.lights;
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

void GameWorld::simulateGameWorld(GMfloat elapsed)
{
	D(d);
	d.character->simulation();
	d.physics->simulate();
	d.ellapsed += elapsed;
}

PhysicsWorld* GameWorld::physicsWorld()
{
	D(d);
	return d.physics;
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

GMfloat GameWorld::getElapsed()
{
	D(d);
	return d.ellapsed;
}

void GameWorld::createPainterForObject(GameObject* obj)
{
	D(d);
	GameMachine* gm = d.gameMachine;
	IFactory* factory = gm->getFactory();
	IGraphicEngine* engine = gm->getGraphicEngine();
	ObjectPainter* painter;
	factory->createPainter(engine, obj->getObject(), &painter);
	ASSERT(!obj->getObject()->getPainter());
	obj->getObject()->setPainter(painter);
}