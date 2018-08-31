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

void GMPhysicsShape::setShape(AUTORELEASE btCollisionShape* shape)
{
	D(d);
	if (d->shape)
		GM_delete(d->shape);
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

void GMPhysicsShapeCreator::createBoxShape(const GMVec3& halfExtents, REF GMPhysicsShapeAsset& physicsShape)
{
	GMFloat4 fs_halfExtents;
	halfExtents.loadFloat4(fs_halfExtents);
	GMPhysicsShape* shape = new GMPhysicsShape();
	shape->setShape(new btBoxShape(btVector3(fs_halfExtents[0], fs_halfExtents[1], fs_halfExtents[2])));
	physicsShape = GMAsset(GMAssetType::PhysicsShape, shape);
}