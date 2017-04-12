#include "stdafx.h"
#include "collisionobjectfactory.h"

CollisionObject CollisionObjectFactory::defaultCamera()
{
	CollisionObject o;
	memset(&o, 0, sizeof(0));

	o.shapeProps.bounding[0] = vmath::vec3(-15, -35, -15);
	o.shapeProps.bounding[1] = vmath::vec3(15, 35, 15);
	o.shapeProps.stepHeight = 18.f;
	return o;
}