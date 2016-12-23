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
	//m_pCallback.reset(NEW_GameWorldRenderCallback);
	dataRef().init();
}

void GameWorld::appendObject(GameObject* obj)
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
	for (int i = dataRef().m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dataRef().m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body)
		{
			btTransform trans;
			body->getMotionState()->getWorldTransform(trans);

			btScalar glTrans[16];
			trans.getOpenGLMatrix(glTrans);

			GameObject* gameObj = dataRef().m_shapes.at(i);
			dataRef().m_pEngine->drawObject(glTrans, gameObj);
		}
	}

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