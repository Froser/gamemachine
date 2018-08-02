#include "stdafx.h"
#include <gmgameworld.h>
#include "gmspritegameobject.h"
#include "gmphysics/gmphysicsworld.h"

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius, const GMVec3& position)
{
	D(d);
	d->radius = radius;
	d->state.position = position;
	d->state.lookAt = GMVec3(0, 0, -1);
	d->limitPitch = Radians(85.f);
}

const GMPositionState& GMSpriteGameObject::getPositionState()
{
	D(d);
	return d->state;
}

void GMSpriteGameObject::action(GMMovement movement, const GMVec3& direction, const GMVec3& rate)
{
	D(d);
	const GMVec3* speed;
	if (movement == GMMovement::Move)
		speed = &d->moveSpeed;
	else if (movement == GMMovement::Jump)
		speed = &d->jumpSpeed;
	else
		GM_ASSERT(false);

	GMSpriteMovement subMovement(direction, rate, *speed, movement);
	d->movements.push_back(subMovement);
}

void GMSpriteGameObject::look(GMfloat pitch, GMfloat yaw)
{
	D(d);
	GMVec3 lookAt = d->state.lookAt;
	GMFloat4 f4_lookAt;
	lookAt.loadFloat4(f4_lookAt);

	// 不考虑roll，把lookAt投影到世界坐标系平面
	GMVec3 lookAt_z = GMVec3(f4_lookAt[0], 0, f4_lookAt[2]);
	// 计算pitch是否超出范围，不考虑roll
	GMfloat calculatedPitch = Asin(f4_lookAt[1]) + pitch;
	if (-d->limitPitch < calculatedPitch && calculatedPitch <= d->limitPitch)
	{
		// 找到lookAt_z垂直的一个向量，使用与世界坐标相同的坐标系
		GMVec3 lookAt_x = GMVec3(1, 0, 0) * GMQuat(GMVec3(0, 0, 1), lookAt_z);
		GMQuat qPitch = Rotate(-pitch, FastNormalize(lookAt_x));
		lookAt = lookAt * qPitch;
	}

	GMQuat qYaw = Rotate(-yaw, GMVec3(0, 1, 0));
	d->state.lookAt = FastNormalize(lookAt * qYaw);
}

void GMSpriteGameObject::update(GMDuration dt)
{
	D(d);
	GMVec3 direction(0), rate(0), moveSpeed(0), jumpSpeed(0);
	bool moved = false, jumped = false, rateAssigned = false;
	for (auto& movement : d->movements)
	{
		if (movement.movement == GMMovement::Move)
		{
			moveSpeed = movement.speed;
			moved = true;
		}
		else if (movement.movement == GMMovement::Jump)
		{
			jumpSpeed = movement.speed;
			jumped = true;
		}
		direction += movement.moveDirection;
		if (!rateAssigned)
		{
			rate = movement.moveRate;
			rateAssigned = true;
		}
	}
	direction = FastNormalize(direction);

	if (moved)
	{
		GMPhysicsMoveArgs args(d->state.lookAt, direction, moveSpeed, rate);
		GMPhysicsWorld* world = getWorld()->getPhysicsWorld();
		world->applyMove(getPhysicsObject(), args);
	}

	if (jumped)
	{
		GMPhysicsMoveArgs args(d->state.lookAt, direction, jumpSpeed, rate);
		GMPhysicsWorld* world = getWorld()->getPhysicsWorld();
		world->applyJump(getPhysicsObject(), args);
	}

	GMFloat4 f4_position;
	GetTranslationFromMatrix(getPhysicsObject()->getMotionStates().transform, f4_position);
	d->state.position.setFloat4(f4_position);
	d->movements.clear();
}