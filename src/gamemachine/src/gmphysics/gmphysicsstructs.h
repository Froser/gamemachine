#ifndef __PHYSICSSTRUCTS_H__
#define __PHYSICSSTRUCTS_H__
#include <gmcommon.h>
#include <map>
#include <linearmath.h>
BEGIN_NS

GM_ALIGNED_STRUCT(GMPhysicsMoveArgs)
{
	GMPhysicsMoveArgs() = default;
	GMPhysicsMoveArgs(const glm::vec3& _lookAt, const glm::vec3& _direction, const glm::vec3& _speed, const glm::vec3& _rate)
		: lookAt(_lookAt)
		, direction(_direction)
		, speed(_speed)
		, rate(_rate)
	{
	}

	glm::vec3 lookAt; //!< 物体朝向
	glm::vec3 direction; //!< 物体移动方向，坐标系相对于物体朝向，采用右手坐标系，与朝向坐标系一致。
	glm::vec3 speed;
	glm::vec3 rate;
};

GM_ALIGNED_STRUCT(GMMotionStates)
{
	glm::vec3 translation = glm::zero<glm::vec3>();
	glm::vec3 velocity = glm::zero<glm::vec3>();
};

GM_PRIVATE_OBJECT(GMPhysicsObject)
{
	GMMotionStates motionStates;
};

//! 表示一个物理对象
/*!
  物理对象拥有一些物理性质，如边界大小等。你不能直接创建它，而是应该根据GMPhysicsWorld::newPhysicsObject()来创建相应的GMPhysicsObject。
  \sa GMPhysicsWorld
*/
class GMPhysicsObject : public GMObject
{
	DECLARE_PRIVATE(GMPhysicsObject)
	friend class GMPhysicsWorld;

protected:
	GMPhysicsObject() = default;

public:
	inline GMMotionStates& motionStates() { D(d); return d->motionStates; }
};

END_NS
#endif
