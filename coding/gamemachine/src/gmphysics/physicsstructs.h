#ifndef __PHYSICSSTRUCTS_H__
#define __PHYSICSSTRUCTS_H__
#include "common.h"
#include <map>
#include "utilities/vmath.h"
BEGIN_NS

class GameObject;

struct ShapeProperties
{
	GMfloat radius;
};

struct MotionProperties
{
	vmath::vec3 translation;
	vmath::vec3 velocity;
};

struct CollisionObject
{
	GameObject* object;
	MotionProperties motions;
	ShapeProperties shapeProps;
};

END_NS
#endif