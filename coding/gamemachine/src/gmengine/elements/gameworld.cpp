#include "stdafx.h"
#include "gameworld.h"
#include "gameobject.h"
#include "btBulletDynamicsCommon.h"
#include "gmdatacore/object.h"
#include "utilities/assert.h"
#include "character.h"
#include "gmengine/controller/graphic_engine.h"
#include "gmengine/elements/gamelight.h"
#include "../controller/gamemachine.h"

GameWorld::GameWorld()
{
	dataRef().init();
}

void GameWorld::initialize()
{
	getGraphicEngine()->initialize(this);
}

void GameWorld::appendObject(AUTORELEASE GameObject* obj)
{
	dataRef().appendObject(obj);
	obj->setWorld(this);
}

void GameWorld::appendLight(AUTORELEASE GameLight* light)
{
	dataRef().appendLight(light);
	light->setWorld(this);
}

std::vector<GameLight*>& GameWorld::getLights()
{
	return dataRef().m_lights;
}

void GameWorld::setMajorCharacter(Character* character)
{
	dataRef().m_character = character;
}

Character* GameWorld::getMajorCharacter()
{
	return dataRef().m_character;
}

void GameWorld::setSky(GameObject* sky)
{
	dataRef().m_sky = sky;
}

GameObject* GameWorld::getSky()
{
	return dataRef().m_sky;
}

void GameWorld::simulateGameWorld(GMfloat elapsed)
{
	dataRef().m_dynamicsWorld->stepSimulation(elapsed, 0);
	dataRef().m_character->simulateCamera();
	dataRef().m_ellapsed += elapsed;
}

void GameWorld::renderGameWorld()
{
	IGraphicEngine* engine = getGraphicEngine();
	engine->newFrame();
	DrawingList list;

	for (auto iter = dataRef().m_shapes.begin(); iter != dataRef().m_shapes.end(); iter++)
	{
		GameObject* gameObj = *iter;
		gameObj->getReadyForRender(list);
	}

	CameraLookAt lookAt;
	Camera::calcCameraLookAt(dataRef().m_character->getPositionState(), &lookAt);
	dataRef().m_character->applyEyeOffset(lookAt);
	engine->updateCameraView(lookAt);

	engine->drawObjects(list);
}

void GameWorld::setGravity(GMfloat x, GMfloat y, GMfloat z)
{
	dataRef().setGravity(x, y, z);
}

IGraphicEngine* GameWorld::getGraphicEngine()
{
	return dataRef().m_gameMachine->getGraphicEngine();
}

void GameWorld::setGameMachine(GameMachine* gm)
{
	dataRef().m_gameMachine = gm;
}

GameMachine* GameWorld::getGameMachine()
{
	return dataRef().m_gameMachine;
}

GMfloat GameWorld::getElapsed()
{
	return dataRef().m_ellapsed;
}