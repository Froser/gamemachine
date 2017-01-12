#include "stdafx.h"
#include "cubegameobject.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"

CubeGameObject::CubeGameObject(const btVector3& extents, const Material eachMaterial[6])
	: m_extents(extents)
	, m_magnification(1)
{
	memcpy(m_eachMaterial, eachMaterial, sizeof(Material) * 6);
	m_collisionExtents = m_extents;
}

CubeGameObject::CubeGameObject(const btVector3& extents, GMfloat magnification, const Material eachMaterial[6])
	: m_extents(extents)
	, m_magnification(magnification <= 0 ? 1 : magnification)
{
	memcpy(m_eachMaterial, eachMaterial, sizeof(Material) * 6);
	m_collisionExtents = m_extents;
}

CubeGameObject::CubeGameObject(const btVector3& extents, AUTORELEASE Object* obj)
	: m_collisionExtents(extents)
{
	setObject(obj);
}

btCollisionShape* CubeGameObject::createCollisionShape()
{
	return new btBoxShape(m_collisionExtents / 2);
}

void CubeGameObject::appendThisObjectToWorld(btDynamicsWorld* world)
{
	initCoreShape();
	RigidGameObject::appendThisObjectToWorld(world);
}

void CubeGameObject::setExtents(const btVector3& extents)
{
	m_extents = extents;
}

btVector3& CubeGameObject::getExtents()
{
	return m_extents;
}

void CubeGameObject::setCollisionExtents(const btVector3& colHalfExtents)
{
	m_collisionExtents = colHalfExtents;
}

void CubeGameObject::initCoreShape()
{
	D(d);
	btScalar x = m_extents.x() * d.localScaling[0] / 2,
		y = m_extents.y() * d.localScaling[1] / 2,
		z = m_extents.z() * d.localScaling[2] / 2;

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
		0, m_magnification,
		0, 0,
		m_magnification, 0,
		m_magnification, m_magnification,

		m_magnification, m_magnification,
		m_magnification, 0,
		0, 0,
		0, m_magnification,

		0, m_magnification,
		m_magnification, m_magnification,
		m_magnification, 0,
		0, 0,

		m_magnification, m_magnification,
		0, m_magnification,
		0, 0,
		m_magnification, 0,

		0, m_magnification,
		0, 0,
		m_magnification, 0,
		m_magnification, m_magnification,

		0, 0,
		0, m_magnification,
		m_magnification, m_magnification,
		m_magnification, 0,
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