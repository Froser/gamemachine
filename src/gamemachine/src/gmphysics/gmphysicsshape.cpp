#include "stdafx.h"
#include "gmphysicsshape.h"
#include "gmbulletincludes.h"
#include <linearmath.h>
#include <gmmodel.h>
#include <gmassets.h>
#include "gmbullethelper.h"

GMPhysicsShape::~GMPhysicsShape()
{
	D(d);
	GM_delete(d->shape);
}

void GMPhysicsShape::setShape(btCollisionShape* shape)
{
	D(d);
	d->shape = shape;
}

btCollisionShape* GMPhysicsShape::getBulletShape()
{
	D(d);
	return d->shape;
}

const btCollisionShape* GMPhysicsShape::getBulletShape() const
{
	D(d);
	return d->shape;
}

//////////////////////////////////////////////////////////////////////////
// Helper

void GMPhysicsShapeCreator::createBoxShape(const glm::vec3& halfExtents, OUT GMPhysicsShape** shape)
{
	GM_ASSERT(shape);
	*shape = new GMPhysicsShape();
	(*shape)->setShape(new btBoxShape(btVector3(halfExtents[0], halfExtents[1], halfExtents[2])));
}