#include "stdafx.h"
#include "collisionobjectfactory.h"

GMCollisionObject CollisionObjectFactory::defaultCamera()
{
	GMCollisionObject o;
	o.shapeProps.bounding[0] = linear_math::Vector3(-15, -35, -15);
	o.shapeProps.bounding[1] = linear_math::Vector3(15, 35, 15);
	o.shapeProps.stepHeight = 18.f;
	o.motions.velocity = linear_math::Vector3(0);
	o.motions.jumpSpeed = linear_math::Vector3(0);
	o.motions.translation = linear_math::Vector3(0);
	o.motions.moveSpeed = 100.f;
	return o;
}