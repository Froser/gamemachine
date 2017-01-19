#include "stdafx.h"
#include "spheregameobject.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "utilities/algorithm.h"

SphereGameObject::SphereGameObject(GMfloat radius, GMfloat slices, GMfloat stacks, const Material& material)
	: m_radius(radius)
	, m_slices(slices)
	, m_stacks(stacks)
	, m_externalObject(false)
{
	memcpy(&m_material, &material, sizeof(Material));
}

SphereGameObject::SphereGameObject(GMfloat radius, AUTORELEASE Object* obj)
	: m_radius(radius)
	, m_externalObject(true)
{
	setObject(obj);
}

btCollisionShape* SphereGameObject::createCollisionShape()
{
	return new btSphereShape(m_radius);
}

void SphereGameObject::initPhysicsAfterCollisionObjectCreated()
{
	if (!m_externalObject)
		initCoreObject();
	RigidGameObject::initPhysicsAfterCollisionObjectCreated();
}

void SphereGameObject::initCoreObject()
{
	D(d);
	Object* coreObj = new Object();
	ChildObject* coreChildObj = nullptr;
	Algorithm::createSphere(m_radius * d.localScaling[0], m_slices, m_stacks, &coreChildObj);
	std::vector<Component*>& components = coreChildObj->getComponents();
	for (auto iter = components.begin(); iter != components.end(); iter++)
	{
		Component* comp = *iter;
		Material& m = comp->getMaterial();
		memcpy(&m, &m_material, sizeof(Material));
	}
	coreObj->append(coreChildObj);
	setObject(coreObj);
}