#ifndef __PHYSICSSTRUCTS_H__
#define __PHYSICSSTRUCTS_H__
#include <gmcommon.h>
#include <map>
#include <linearmath.h>
BEGIN_NS

GM_ALIGNED_STRUCT(GMPhysicsMoveArgs)
{
	GMPhysicsMoveArgs() = default;
	GMPhysicsMoveArgs(const glm::quat& _lookAt, const glm::vec3& _direction, const glm::vec3& _rate)
		: lookAt(_lookAt)
		, direction(_direction)
		, rate(_rate)
	{
	}

	glm::quat lookAt; //!< 物体朝向
	glm::vec3 direction; //!< 物体移动方向，坐标系相对于物体朝向，采用右手坐标系，与朝向坐标系一致。
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
