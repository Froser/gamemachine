#include "stdafx.h"
#include "gameworld.h"
#include "gameobject.h"
#include "btBulletDynamicsCommon.h"
#include "gmdatacore/object.h"
#include "utilities/assert.h"
#include "character.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmengine/elements/gamelight.h"
#include "gmengine/controller/gamemachine.h"
#include <algorithm>
#include "gmengine/controller/factory.h"

GameWorld::GameWorld()
{
	D(d);
	d.collisionConfiguration.reset(new btDefaultCollisionConfiguration());
	d.dispatcher.reset(new btCollisionDispatcher(d.collisionConfiguration));
	d.overlappingPairCache.reset(new btDbvtBroadphase());
	d.solver.reset(new btSequentialImpulseConstraintSolver);
	d.ghostPairCallback.reset(new btGhostPairCallback());
	d.dynamicsWorld.reset(new btDiscreteDynamicsWorld(d.dispatcher, d.overlappingPairCache, d.solver, d.collisionConfiguration));
	d.dynamicsWorld->getBroadphase()->getOverlappingPairCache()->setInternalGhostPairCallback(d.ghostPairCallback);
}

GameWorld::~GameWorld()
{
	D(d);
	for (int i = d.dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = d.dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		d.dynamicsWorld->removeCollisionObject(obj);
		delete obj;
	}

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

void GameWorld::appendObject(AUTORELEASE GameObject* obj)
{
	D(d);
	if (std::find(d.shapes.begin(), d.shapes.end(), obj) != d.shapes.end())
		return;

	obj->appendObjectToWorld(d.dynamicsWorld);
	d.shapes.push_back(obj);

	// 创建一个Painter
	createPainterForObject(obj);

	// Painter在transfer之后，会删除Object顶点数据，所以放到最后transfer
	ObjectPainter* painter = obj->getObject()->getPainter();
	if (painter)
		painter->transfer();

	obj->setWorld(this);
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

void GameWorld::setSky(GameObject* sky)
{
	D(d);
	d.sky = sky;
}

GameObject* GameWorld::getSky()
{
	D(d);
	return d.sky;
}

void GameWorld::simulateGameWorld(GMfloat elapsed)
{
	D(d);
	d.dynamicsWorld->stepSimulation(elapsed, 0);
	d.character->simulateCamera();
	d.ellapsed += elapsed;
}

void GameWorld::renderGameWorld()
{
	D(d);
	IGraphicEngine* engine = getGraphicEngine();
	engine->newFrame();
	DrawingList list;

	for (auto iter = d.shapes.begin(); iter != d.shapes.end(); iter++)
	{
		GameObject* gameObj = *iter;
		gameObj->getReadyForRender(list);
	}

	d.character->updateLookAt();
	CameraLookAt& lookAt = d.character->getLookAt();
	d.character->applyEyeOffset(lookAt);
	engine->updateCameraView(lookAt);

	engine->drawObjects(list);
}

void GameWorld::setGravity(GMfloat x, GMfloat y, GMfloat z)
{
	D(d);
	d.dynamicsWorld->setGravity(btVector3(x, y, z));
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

GameObject* GameWorld::findGameObjectById(GMuint id)
{
	D(d);
	for (auto iter = d.shapes.begin(); iter != d.shapes.end(); iter++)
	{
		if ((*iter)->getId() == id)
			return (*iter);
	}

	return nullptr;
}