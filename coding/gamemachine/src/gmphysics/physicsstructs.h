#ifndef __PHYSICSSTRUCTS_H__
#define __PHYSICSSTRUCTS_H__
#include "common.h"
#include <map>
#include "utilities/vector.h"
#include "utilities/linearmath.h"
#include "utilities/vector.h"
BEGIN_NS

typedef GMint Command;

typedef std::map<Command, AlignedVector<linear_math::Vector3> > CommandParams;

#define USELESS_PARAM 0
#define CMD_NONE 0x0000
#define CMD_MOVE 0x0001
#define CMD_JUMP 0x0002

class GameObject;
struct ShapeProperties
{
	GMfloat stepHeight;
	linear_math::Vector3 bounding[2]; //最小边界和最大边界
};

struct MotionProperties
{
	linear_math::Vector3 translation;
	linear_math::Vector3 velocity;
	linear_math::Vector3 jumpSpeed;
	GMfloat moveSpeed;
};

struct CollisionObject
{
	GameObject* object;
	MotionProperties motions;
	ShapeProperties shapeProps;
};

END_NS
#endif
