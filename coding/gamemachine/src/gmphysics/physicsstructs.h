#ifndef __PHYSICSSTRUCTS_H__
#define __PHYSICSSTRUCTS_H__
#include "common.h"
#include <map>
#include "utilities/vector.h"
#include "utilities/linearmath.h"
#include "utilities/vector.h"
BEGIN_NS

typedef GMint Command;

class CommandVector3
{
public:
	CommandVector3() {}

	CommandVector3(GMfloat x, GMfloat y, GMfloat z)
	{
		p[0] = x;
		p[1] = y;
		p[2] = z;
	}

	GMfloat& operator[](GMint i)
	{
		return p[i];
	}

private:
	GMfloat p[3];
};

typedef std::map<Command, AlignedVector<CommandVector3> > CommandParams;

#define USELESS_PARAM 0
#define CMD_NONE 0x0000
#define CMD_MOVE 0x0001
#define CMD_JUMP 0x0002

class GameObject;
struct ShapeProperties
{
	GM_DECLARE_ALIGNED_ALLOCATOR();

	linear_math::Vector3 bounding[2]; //最小边界和最大边界
	GMfloat stepHeight;
};

struct MotionProperties
{
	GM_DECLARE_ALIGNED_ALLOCATOR();

	linear_math::Vector3 translation;
	linear_math::Vector3 velocity;
	linear_math::Vector3 jumpSpeed;
	GMfloat moveSpeed;
};

struct CollisionObject
{
	GM_DECLARE_ALIGNED_ALLOCATOR();

	MotionProperties motions;
	ShapeProperties shapeProps;
	GameObject* object;
};

END_NS
#endif
