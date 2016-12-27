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

void GameWorld::simulateGameWorld(GMint fps)
{
	dataRef().m_dynamicsWorld->stepSimulation(1.f / fps);
	dataRef().m_character->simulateCamera();
}

void GameWorld::renderGameWorld()
{
	dataRef().m_pEngine->newFrame();
	DrawingList list;

	for (auto iter = dataRef().m_shapes.begin(); iter != dataRef().m_shapes.end(); iter++)
	{
		GameObject* gameObj = *iter;
		btCollisionObject* obj = gameObj->getCollisionObject();
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body)
		{
			btTransform trans;
			body->getMotionState()->getWorldTransform(trans);

			btScalar glTrans[16];
			trans.getOpenGLMatrix(glTrans);

			DrawingItem item;
			memcpy(item.trans, glTrans, sizeof(glTrans));
			item.gameObject = gameObj;
			list.push_back(item);
		}
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