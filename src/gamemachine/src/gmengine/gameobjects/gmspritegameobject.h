#ifndef __GMSPRITEGAMEOBJECT_H__
#define __GMSPRITEGAMEOBJECT_H__
#include <gmcommon.h>
#include <gmphysicsworld.h>
#include <gmcamera.h>
#include "gmgameobject.h"
BEGIN_NS

GM_ALIGNED_STRUCT(GMSpriteMovement)
{
	GMSpriteMovement() = default;

	GMSpriteMovement(const GMVec3& dir, const GMVec3& rate, const GMVec3& _speed, GMMovement m)
		: moveDirection(dir)
		, moveRate(rate)
		, speed(_speed)
		, movement(m)
	{}

	GMVec3 moveDirection;
	GMVec3 moveRate;
	GMVec3 speed;
	GMMovement movement;
};

GM_ALIGNED_STRUCT(GMPositionState)
{
	GMVec3 position;
	GMVec3 lookAt;
};

GM_PRIVATE_OBJECT(GMSpriteGameObject)
{
	GMfloat radius;
	GMfloat limitPitch;
	GMPositionState state;
	AlignedVector<GMSpriteMovement> movements;
	GMVec3 moveSpeed = Zero<GMVec3>();
	GMVec3 jumpSpeed = Zero<GMVec3>();
};

class GMSpriteGameObject : public GMGameObject
{
	GM_DECLARE_PRIVATE_AND_BASE(GMSpriteGameObject, GMGameObject)

public:
	GMSpriteGameObject(GMfloat radius, const GMVec3& position = Zero<GMVec3>());

public:
	virtual void simulate() override;
	virtual void updateAfterSimulate() override;

public:
	//! 表示精灵对象执行一个动作。
	/*!
	  精灵对象执行一个动作。动作的效果由本对象一些物理属性合成。
	  \param movement 动作的类型，如跳跃、移动等。
	  \param direction 动作的方向。动作方向所采用的坐标系为精灵朝向的坐标系，并采用右手坐标系。即：精灵面朝z轴正方向，精灵左手为x轴正方向，头顶方向为y轴正方向。
	  \param rate 比率。在计算移动的时位移的折扣。如用手柄时，手柄摇杆有个范围，此时通过比率来决定要位移原本要位移的多少。如果摇杆打到尽头，则可以认为比率为1。
	*/
	void action(GMMovement movement, const GMVec3& direction = GMVec3(), const GMVec3& rate = GMVec3(1));

	//! 表示精灵朝着某个方向变化。
	/*!
	  此方法表示一个变化量。它会在原有的朝向上进行叠加，而不是将朝向调整为某一个值。
	  \param pitchDegree 俯仰角，绕视角坐标系x轴旋转的弧度数。
	  \param yawDegree 偏航角，绕视角坐标系y轴旋转的弧度数。
	*/
	void look(GMfloat pitch, GMfloat yaw);
	const GMPositionState& getPositionState();

public:
	inline void setMoveSpeed(const GMVec3& speed) { D(d); d->moveSpeed = speed; }
	inline void setJumpSpeed(const GMVec3& speed) { D(d); d->jumpSpeed = speed; }
};


END_NS
#endif