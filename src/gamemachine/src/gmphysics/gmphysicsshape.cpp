#include "stdafx.h"
#include "gmphysicsshape.h"
#include "gmbulletincludes.h"
#include <linearmath.h>
#include <gmmodel.h>
#include <gmassets.h>
#include "gmbullethelper.h"

void GMPhysicsShape::setShape(AUTORELEASE btCollisionShape* shape)
{
	D(d);
	d->shape.reset(shape);
}

btCollisionShape* GMPhysicsShape::getBulletShape()
{
	D(d);
	return d->shape.get();
}

const btCollisionShape* GMPhysicsShape::getBulletShape() const
{
	D(d);
	return d->shape.get();
}

//////////////////////////////////////////////////////////////////////////
// Helper

void GMPhysicsShapeCreator::createBoxShape(const GMVec3& halfExtents, OUT GMPhysicsShape** shape)
{
	GM_ASSERT(shape);
	GMFloat4 fs_halfExtents;
	halfExtents.loadFloat4(fs_halfExtents);
	*shape = new GMPhysicsShape();
	(*shape)->setShape(new btBoxShape(btVector3(fs_halfExtents[0], fs_halfExtents[1], fs_halfExtents[2])));
}