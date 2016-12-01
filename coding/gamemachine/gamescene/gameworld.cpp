#include "stdafx.h"
#include "gameworld.h"
#include "gameobject.h"
#include "btBulletDynamicsCommon.h"
#include "core/objstruct.h"
#include "utilities/assert.h"

void GLGameWorldRenderCallback::renderObject(btScalar* trans, GameObject* obj)
{
	glPushMatrix();
	glMultMatrixf(trans);
	glCallList(obj->getObject()->m_objectId);
	glPopMatrix();
}

void GLGameWorldRenderCallback::getBufferedObject(GameObject* obj, Object** out)
{
	GMuint id = glGenLists(1);
	glNewList(id, GL_COMPILE);
	glPushMatrix();
	obj->drawObject();
	glPopMatrix();
	glEndList();

	if (*out)
	{
		TextureMap tm;
		*out = new Object(id, tm);
	}
}

GameWorld::GameWorld()
{
	m_pCallback.reset(NEW_GameWorldRenderCallback);
	dataRef().init();
}

void GameWorld::appendObject(GameObject* obj)
{
	Object* coreObj;
	m_pCallback->getBufferedObject(obj, &coreObj);
	obj->setObject(coreObj);

	dataRef().appendObject(obj);
}

void GameWorld::renderGameWorld(GMint fps)
{
	dataRef().m_dynamicsWorld->stepSimulation(1.f / fps);

	for (int i = dataRef().m_dynamicsWorld->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dataRef().m_dynamicsWorld->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		btTransform trans;
		body->getMotionState()->getWorldTransform(trans);

		btScalar glTrans[16];
		trans.getOpenGLMatrix(glTrans);

		GameObject* gameObj = dataRef().m_shapes.at(i);
		m_pCallback->renderObject(glTrans, gameObj);
	}
}