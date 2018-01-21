#include "stdafx.h"
#include <gmgameworld.h>
#include "gmspritegameobject.h"
#include "gmphysics/gmphysicsworld.h"

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius, const glm::vec3& position)
{
	D(d);
	d->radius = radius;
	d->state.position = position;
	d->state.lookAt = glm::vec3(0, 0, 1);
	d->limitPitch = glm::radians(85.f);
}

const GMPositionState& GMSpriteGameObject::getPositionState()
{
	D(d);
	return d->state;
}

void GMSpriteGameObject::action(GMMovement movement, const glm::vec3& direction, const glm::vec3& rate)
{
	D(d);
	const glm::vec3* speed;
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
	glm::vec3 lookAt = d->state.lookAt;
	// 不考虑roll，把lookAt投影到世界坐标系平面
	glm::vec3 lookAt_z = glm::vec3(lookAt[0], 0, lookAt[2]);
	// 计算pitch是否超出范围，不考虑roll
	GMfloat calculatedPitch = glm::asin(d->state.lookAt[1]) + pitch;
	if (-d->limitPitch < calculatedPitch && calculatedPitch <= d->limitPitch)
	{
		// 找到lookAt_z垂直的一个向量，使用与世界坐标相同的坐标系
		glm::vec3 lookAt_x = glm::quat(glm::vec3(0, 0, 1), lookAt_z) * glm::vec3(1, 0, 0);
		glm::quat qPitch = glm::rotate(glm::identity<glm::quat>(), -pitch, glm::fastNormalize(lookAt_x));
		lookAt = qPitch * lookAt;
	}

	glm::quat qYaw = glm::rotate(glm::identity<glm::quat>(), -yaw, glm::vec3(0, 1, 0));
	d->state.lookAt = glm::fastNormalize(qYaw * lookAt);
}

void GMSpriteGameObject::simulate()
{
	D(d);
	glm::vec3 direction(0), rate(0), moveSpeed(0), jumpSpeed(0);
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
	direction = glm::fastNormalize(direction);

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
}

void GMSpriteGameObject::updateAfterSimulate()
{
	D(d);
	glm::getTranslationFromMatrix(getPhysicsObject()->getMotionStates().transform, glm::value_ptr(d->state.position));
	d->movements.clear();
}