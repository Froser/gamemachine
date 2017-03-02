#ifndef __PHYSICSWORLD_PRIVATE_H__
#define __PHYSICSWORLD_PRIVATE_H__
#include "common.h"
#include "utilities/vmath.h"
#include "physicsstruct.h"
BEGIN_NS

struct PhysicsWorldPrivate
{
	PhysicsWorldPrivate();

	CollisionTree tree;
	CollisionObject camera;

	vmath::vec3 gravity;
};

END_NS
#endif