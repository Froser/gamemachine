#ifndef __PHYSICSSTRUCTS_H__
#define __PHYSICSSTRUCTS_H__
#include <gmcommon.h>
#include <map>
#include <linearmath.h>
BEGIN_NS

GM_ALIGNED_STRUCT(GMPhysicsMoveArgs)
{
	GMPhysicsMoveArgs(const glm::quat& _lookAt, const glm::vec3& _direction, const glm::vec3& _rate)
		: lookAt(_lookAt)
		, direction(_direction)
		, rate(_rate)
	{
	}

	glm::quat lookAt;
	glm::vec3 direction;
	glm::vec3 rate;
};

GM_ALIGNED_STRUCT(GMShapeProperties)
{
	glm::vec3 bounding[2]; //最小边界和最大边界
	GMfloat stepHeight;
};

GM_ALIGNED_STRUCT(GMMotionProperties)
{
	glm::vec3 translation;
	glm::vec3 velocity;
	glm::vec3 jumpSpeed;
	glm::vec3 moveSpeed;
};

GM_ALIGNED_STRUCT(GMPhysicsObject)
{
	GMMotionProperties motions;
	GMShapeProperties shapeProps;
};

END_NS
#endif
