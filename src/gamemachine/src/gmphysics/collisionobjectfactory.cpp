#include "stdafx.h"
#include "collisionobjectfactory.h"

GMCollisionObject CollisionObjectFactory::defaultCamera()
{
	GMCollisionObject o;
	o.shapeProps.bounding[0] = glm::vec3(-15, -35, -15);
	o.shapeProps.bounding[1] = glm::vec3(15, 35, 15);
	o.shapeProps.stepHeight = 18.f;
	o.motions.velocity = glm::vec3(0);
	o.motions.jumpSpeed = glm::vec3(0);
	o.motions.translation = glm::vec3(0);
	o.motions.moveSpeed = glm::vec3(100.f);
	return o;
}