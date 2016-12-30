#include "stdafx.h"
#include "gameworld.h"
#include "gameobject.h"
#include "btBulletDynamicsCommon.h"
#include "gmdatacore/object.h"
#include "utilities/assert.h"
#include "character.h"
#include "gmengine/controller/graphic_engine.h"

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

void GameWorld::setMajorCharacter(Character* character)
{
	dataRef().m_character = character;
	appendObject(character);
}

Character* GameWorld::getMajorCharacter()
{
	return dataRef().m_character;
}

void GameWorld::setSky(GameObject* sky)
{
	dataRef().m_sky = sky;
	appendObject(sky);
}

GameObject* GameWorld::getSky()
{
	return dataRef().m_sky;
}

void GameWorld::simulateGameWorld(GMfloat elapsed)
{
	dataRef().m_dynamicsWorld->stepSimulation(elapsed, 0);
	dataRef().m_character->simulateCamera();
}

void GameWorld::renderGameWorld()
{
	dataRef().m_pEngine->newFrame();
	DrawingList list;

	for (auto iter = dataRef().m_shapes.begin(); iter != dataRef().m_shapes.end(); iter++)
	{
		GameObject* gameObj = *iter;
		gameObj->getReadyForRender(list);
	}

	dataRef().m_pEngine->drawObjects(list);
	dataRef().m_pEngine->updateCameraView(dataRef().m_character->getCamera());
}

void GameWorld::setGravity(GMfloat x, GMfloat y, GMfloat z)
{
	dataRef().setGravity(x, y, z);
}

IGraphicEngine* GameWorld::getGraphicEngine()
{
	return dataRef().m_pEngine;
}