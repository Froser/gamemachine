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

			btVector3 scaling = obj->getCollisionShape()->getLocalScaling();
			vmath::mat4 T(
				vmath::vec4(glTrans[0], glTrans[1], glTrans[2], glTrans[3]),
				vmath::vec4(glTrans[4], glTrans[5], glTrans[6], glTrans[7]),
				vmath::vec4(glTrans[8], glTrans[9], glTrans[10], glTrans[11]),
				vmath::vec4(glTrans[12], glTrans[13], glTrans[14], glTrans[15])
			);
			vmath::mat4 S(
				vmath::vec4(scaling[0], 0, 0, 0),
				vmath::vec4(0, scaling[1], 0, 0),
				vmath::vec4(0, 0, scaling[2], 0),
				vmath::vec4(0, 0, 0, 1)
			);

			DrawingItem item;
			memcpy(item.trans, T * S, sizeof(T));
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