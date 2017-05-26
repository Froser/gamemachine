#ifndef __PHYSICSSTRUCTS_H__
#define __PHYSICSSTRUCTS_H__
#include "common.h"
#include <map>
#include "foundation/vector.h"
#include "foundation/linearmath.h"
#include "foundation/vector.h"
BEGIN_NS

typedef GMint Command;

GM_ALIGNED_16(class) CommandVector3 : public GMObject
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

GM_ALIGNED_STRUCT(ShapeProperties)
{
	linear_math::Vector3 bounding[2]; //最小边界和最大边界
	GMfloat stepHeight;
};

GM_ALIGNED_STRUCT(MotionProperties)
{
	linear_math::Vector3 translation;
	linear_math::Vector3 velocity;
	linear_math::Vector3 jumpSpeed;
	GMfloat moveSpeed;
};

GM_ALIGNED_STRUCT(CollisionObject)
{
	MotionProperties motions;
	ShapeProperties shapeProps;
	GameObject* object;
};

END_NS
#endif
