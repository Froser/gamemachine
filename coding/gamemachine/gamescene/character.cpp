#include "stdafx.h"
#include "character.h"
#include "btBulletCollisionCommon.h"
#include "utilities/assert.h"

Character::Character(CharacterModelType type, const btTransform& position, btScalar radius)
	: m_type(type)
	, m_radius(radius)
{
	setTransform(position);
}

Character::Character(CharacterModelType type, const btTransform& position, const btVector3& extents)
	: m_extents(extents)
{
	setTransform(position);
}

btMotionState* Character::createMotionState()
{
	return new btDefaultMotionState(getTransform());
}

void gm::Character::drawObject()
{
	
}

btCollisionShape* Character::createCollisionShape()
{
	switch (m_type)
	{
	case gm::Character::Sphere:
		return new btSphereShape(m_radius);
	case gm::Character::Box:
		return new btBoxShape(m_extents);
	default:
		ASSERT(false);
		break;
	}
}
