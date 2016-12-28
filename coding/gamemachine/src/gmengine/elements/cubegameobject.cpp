#include "stdafx.h"
#include "cubegameobject.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

CubeGameObject::CubeGameObject(const btVector3& extents, const btTransform& position, const Material eachMaterial[6])
	: m_extents(extents)
{
	memcpy(m_eachMaterial, eachMaterial, sizeof(Material) * 6);
	setTransform(position);
	createCoreObject();
}

btCollisionShape* CubeGameObject::createCollisionShape()
{
	return new btBoxShape(getExtents());
}

void CubeGameObject::setExtents(const btVector3& extents)
{
	m_extents = extents;
}

btVector3& CubeGameObject::getExtents()
{
	return m_extents;
}

void CubeGameObject::appendObjectToWorld(btDynamicsWorld* world)
{
	btMotionState* motionState = new btDefaultMotionState(getTransform());
	btRigidBody::btRigidBodyConstructionInfo rbInfo(getMass(), motionState, getCollisionShape(), getLocalInertia());
	btRigidBody* rigidObj = new btRigidBody(rbInfo);
	world->addRigidBody(rigidObj);
	setCollisionObject(rigidObj);
}

void CubeGameObject::createCoreObject()
{
	btScalar x = m_extents.x(),
		y = m_extents.y(),
		z = m_extents.z();

	GMfloat vertices[] = {
		//Front
		-x, y, z, 1,
		-x, -y, z, 1,
		x, -y, z, 1,
		x, y, z, 1,

		//Back
		-x, y, -z, 1,
		-x, -y, -z, 1,
		x, -y, -z, 1,
		x, y, -z, 1,

		//Left
		-x, y, -z, 1,
		-x, y, z, 1,
		-x, -y, z, 1,
		-x, -y, -z, 1,

		//Right
		x, y, -z, 1,
		x, y, z, 1,
		x, -y, z, 1,
		x, -y, -z, 1,

		//Up
		-x, y, -z, 1,
		-x, y, z, 1,
		x, y, z, 1,
		x, y, -z, 1,

		//Down
		-x, -y, -z, 1,
		-x, -y, z, 1,
		x, -y, z, 1,
		x, -y, -z, 1,
	};

	GMfloat normals[] = {
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,

		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,

		-1, 0, 0, 1,
		-1, 0, 0, 1,
		-1, 0, 0, 1,
		-1, 0, 0, 1,

		1, 0, 0, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,
		1, 0, 0, 1,

		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,
		0, 0, 1, 1,

		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,
		0, 0, -1, 1,
	};

	GMfloat uvs[] = {
		0, 0,
		0, 1,
		1, 1,
		1, 0,

		0, 0,
		0, 1,
		1, 1,
		1, 0,

		0, 0,
		0, 1,
		1, 1,
		1, 0,

		0, 0,
		0, 1,
		1, 1,
		1, 0,

		0, 0,
		0, 1,
		1, 1,
		1, 0,

		0, 0,
		0, 1,
		1, 1,
		1, 0,
	};

	Object* coreObj = getObject();
	for (GMuint i = 0; i < 4 * 6 * 4; i++)
	{
		if (i % 16 == 0)
		{
			Component* c = new Component();
			c->setEdgeCountPerPolygon(4);
			memcpy(&c->getMaterial(), &m_eachMaterial[(int)(i / 16)], sizeof(Material));
			c->setOffset(i / 4);
			coreObj->appendComponent(c, 4);
		}
		coreObj->vertices().push_back(vertices[i]);
		coreObj->normals().push_back(normals[i]);

		if (i < 48)
			coreObj->uvs().push_back(uvs[i]);
	}
}