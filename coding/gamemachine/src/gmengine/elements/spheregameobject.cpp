#include "stdafx.h"
#include "spheregameobject.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "utilities/algorithm.h"

SphereGameObject::SphereGameObject(GMfloat radius, GMfloat slices, GMfloat stacks, const Material& material)
	: m_radius(radius)
	, m_slices(slices)
	, m_stacks(stacks)
{
	memcpy(&m_material, &material, sizeof(Material));
	createCoreObject();
}

SphereGameObject::SphereGameObject(GMfloat radius, AUTORELEASE Object* obj)
	: m_radius(radius)
{
	setObject(obj);
}

btCollisionShape* SphereGameObject::createCollisionShape()
{
	return new btSphereShape(m_radius);
}

void SphereGameObject::appendObjectToWorld(btDynamicsWorld* world)
{
	btMotionState* motionState = new btDefaultMotionState(getTransform());
	btRigidBody::btRigidBodyConstructionInfo rbInfo(getMass(), motionState, getCollisionShape(), getLocalInertia());
	btRigidBody* rigidObj = new btRigidBody(rbInfo);
	world->addRigidBody(rigidObj);
	setCollisionObject(rigidObj);
}

void SphereGameObject::createCoreObject()
{
	Object* coreObj = nullptr;
	Algorithm::createSphere(m_radius, m_slices, m_stacks, &coreObj);
	setObject(coreObj);
	std::vector<Component*>& components = coreObj->getComponents();
	for (auto iter = components.begin(); iter != components.end(); iter++)
	{
		Component* comp = *iter;
		Material& m = comp->getMaterial();
		memcpy(&m, &m_material, sizeof(Material));
	}
}