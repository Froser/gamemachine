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
	GMSpriteMovement subMovement(direction, rate, movement);
	d->movements.push_back(subMovement);
}

void GMSpriteGameObject::look(GMfloat pitch, GMfloat yaw)
{
	D(d);
	glm::vec3 lookAt = d->state.lookAt;
	// 不考虑roll，把lookAt投影到世界坐标系平面
	glm::vec3 lookAt_z = glm::vec3(lookAt[0], 0, lookAt[2]);
	// 找到lookAt_z垂直的一个向量，使用与世界坐标相同的坐标系
	glm::vec3 lookAt_x = glm::quat(glm::vec3(0, 0, 1), lookAt_z) * glm::vec3(1, 0, 0);
	// 沿着视角方向的x轴旋转 (pitch)
	glm::quat qPitch = glm::rotate(glm::identity<glm::quat>(), -pitch, glm::fastNormalize(lookAt_x));
	glm::quat qYaw = glm::rotate(glm::identity<glm::quat>(), -yaw, glm::vec3(0, 1, 0));
	// 计算pitch是否超出范围，不考虑roll
	GMfloat currentPitch = glm::asin(d->state.lookAt[1]);
	GMfloat calculatedPitch = currentPitch + pitch;
	if (-d->limitPitch < calculatedPitch && calculatedPitch <= d->limitPitch)
		lookAt = qPitch * lookAt;

	d->state.lookAt = glm::fastNormalize(qYaw * lookAt);
}

void GMSpriteGameObject::simulate()
{
	D(d);
	glm::vec3 direction(0), rate(0);
	bool moved = false, jumped = false;
	for (auto& movement : d->movements)
	{
		if (movement.movement == GMMovement::Move)
			moved = true;
		else if (movement.movement == GMMovement::Jump)
			jumped = true;
		direction += movement.moveDirection;
		rate += movement.moveRate;
	}
	direction = glm::fastNormalize(direction);
	rate = glm::fastNormalize(rate);

	if (moved)
	{
		GMPhysicsWorld* world = getWorld()->getPhysicsWorld();
		GMPhysicsMoveArgs args(d->state.lookAt, direction, rate);
		world->applyMove(getPhysicsObject(), args);
	}

	if (jumped)
	{
		GMPhysicsWorld* world = getWorld()->getPhysicsWorld();
		world->applyJump(getPhysicsObject());
	}
}

void GMSpriteGameObject::updateAfterSimulate()
{
	D(d);
	d->state.position = getPhysicsObject()->getMotions().translation;
	d->movements.clear();
}