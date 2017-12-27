#include "stdafx.h"
#include <gmgameworld.h>
#include "gmspritegameobject.h"
#include "gmphysics/gmphysicsworld.h"

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius)
{
	D(d);
	d->radius = radius;
	d->state.position = glm::vec3(0);
	d->state.lookAt = glm::vec3(0, 0, 1);
	d->pitchLimit = HALF_PI - glm::radians(3.f);
}

void GMSpriteGameObject::setJumpSpeed(const glm::vec3& jumpSpeed)
{
	D(d);
	getPhysicsObject().motions.jumpSpeed = jumpSpeed;
}

void GMSpriteGameObject::setMoveSpeed(const glm::vec3& moveSpeed)
{
	D(d);
	getPhysicsObject().motions.moveSpeed = moveSpeed;
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

	// 首先，沿着x轴旋转 (pitch)
	glm::quat x = glm::rotate(glm::identity<glm::quat>(), -pitch, glm::vec3(1, 0, 0));
	// 再沿着y轴转 (yaw)
	glm::quat y = glm::rotate(glm::identity<glm::quat>(), -yaw, glm::vec3(0, 1, 0));

	lookAt = y * x * lookAt;
	d->state.lookAt = glm::fastNormalize(lookAt);
}

void GMSpriteGameObject::setPitchLimit(GMfloat limit)
{
	D(d);
	d->pitchLimit = HALF_PI - glm::radians(limit);
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
		GMPhysicsWorld* world = getWorld()->physicsWorld();
		GMPhysicsMoveArgs args(d->state.lookAt, direction, rate);
		world->applyMove(getPhysicsObject(), args);
	}

	if (jumped)
	{
		GMPhysicsWorld* world = getWorld()->physicsWorld();
		world->applyJump(getPhysicsObject());
	}
}

void GMSpriteGameObject::updateAfterSimulate()
{
	D(d);
	d->state.position = getPhysicsObject().motions.translation;
	d->movements.clear();
}