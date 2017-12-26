#include "stdafx.h"
#include <gmgameworld.h>
#include "gmspritegameobject.h"
#include "gmphysics/gmphysicsworld.h"

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius)
{
	D(d);
	d->radius = radius;
	d->state.position = glm::vec3(0);
	d->state.lookAt = glm::quat(glm::vec3(0, 0, 0));
	d->pitchLimitRadius = HALF_PI - RAD(3);
}


void GMSpriteGameObject::onAppendingObjectToWorld()
{
	/// TODO
	GM_ASSERT(false);
	//sendMoveCommand();
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

void GMSpriteGameObject::lookRight(GMfloat degree)
{
	D(d);
	//d->state.yaw += RAD(degree);
}

void GMSpriteGameObject::lookUp(GMfloat degree)
{
	D(d);
	//d->state.pitch += RAD(degree);
	//if (d->state.pitch > d->pitchLimitRadius)
	//	d->state.pitch = d->pitchLimitRadius;
	//else if (d->state.pitch < -d->pitchLimitRadius)
	//	d->state.pitch = -d->pitchLimitRadius;
}

void GMSpriteGameObject::setPitchLimitDegree(GMfloat deg)
{
	D(d);
	d->pitchLimitRadius = HALF_PI - RAD(deg);
}

void GMSpriteGameObject::simulate()
{
	D(d);
	glm::vec3 directions(0), rate(0);
	bool moved = false, jumped = false;
	for (auto& movement : d->movements)
	{
		if (movement.movement == GMMovement::Move)
			moved = true;
		else if (movement.movement == GMMovement::Jump)
			jumped = true;
		directions += movement.moveDirection;
		rate += movement.moveRate;
	}
	directions = glm::fastNormalize(directions);
	rate = glm::fastNormalize(rate);

	if (moved)
		sendMoveCommand(directions, rate);

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

void GMSpriteGameObject::sendMoveCommand(const glm::vec3& direction, const glm::vec3& rate)
{
	D(d);
	GMPhysicsWorld* world = getWorld()->physicsWorld();
	GMPhysicsMoveArgs args (d->state.lookAt, direction, rate);
	world->applyMove(getPhysicsObject(), args);
}