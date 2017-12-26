#include "stdafx.h"
#include <gmgameworld.h>
#include "gmspritegameobject.h"
#include "gmphysics/gmphysicsworld.h"

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius)
{
	D(d);
	d->radius = radius;
	d->state.position = glm::vec3(0);
	d->state.pitch = 0;
	d->state.yaw = 0;
	clearMoveArgs();
	d->pitchLimitRadius = HALF_PI - RAD(3);
}


void GMSpriteGameObject::onAppendingObjectToWorld()
{
	sendMoveCommand();
}

void GMSpriteGameObject::setJumpSpeed(const glm::vec3& jumpSpeed)
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.jumpSpeed = jumpSpeed;
}

void GMSpriteGameObject::setMoveSpeed(const glm::vec3& moveSpeed)
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.moveSpeed = moveSpeed;
}

const GMPositionState& GMSpriteGameObject::getPositionState()
{
	D(d);
	return d->state;
}

void GMSpriteGameObject::action(GMMovement movement, const glm::vec3& direction, const glm::vec3& rate)
{
	D(d);
	GMSpriteSubMovement subMovement(direction, rate, movement);
	d->movements.push_back(subMovement);
}

void GMSpriteGameObject::lookRight(GMfloat degree)
{
	D(d);
	d->state.yaw += RAD(degree);
}

void GMSpriteGameObject::lookUp(GMfloat degree)
{
	D(d);
	d->state.pitch += RAD(degree);
	if (d->state.pitch > d->pitchLimitRadius)
		d->state.pitch = d->pitchLimitRadius;
	else if (d->state.pitch < -d->pitchLimitRadius)
		d->state.pitch = -d->pitchLimitRadius;
}

void GMSpriteGameObject::setPitchLimitDegree(GMfloat deg)
{
	D(d);
	d->pitchLimitRadius = HALF_PI - RAD(deg);
}

void GMSpriteGameObject::simulate()
{
	D(d);
	clearMoveArgs();

	bool moved = false, jumped = false;
	for (auto& movement : d->movements)
	{
		if (movement.movement == GMMovement::Move)
			moved = true;
		else if (movement.movement == GMMovement::Jump)
			jumped = true;
	}

	if (moved)
		sendMoveCommand();

	if (jumped)
	{
		GMPhysicsWorld* world = getWorld()->physicsWorld();
		GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
		if (c)
		{
			CommandParams cmdParams = GMPhysicsWorld::makeCommand(CMD_JUMP, nullptr, 0);
			world->sendCommand(c, cmdParams);
		}
		else
		{
			gm_error(_L("cannot found character in physics world"));
		}
	}
}

void GMSpriteGameObject::updateAfterSimulate()
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		d->state.position = c->motions.translation;
	else
		gm_error(_L("cannot found character in physics world"));

	d->movements.clear();
}

void GMSpriteGameObject::sendMoveCommand()
{
	D(d);
	//GMPhysicsWorld* pw = getWorld()->physicsWorld();
	//GMCommandVector3 args[] = {
	//	GMCommandVector3(d->state.pitch, d->state.yaw, USELESS_PARAM),
	//	d->moveCmdArgFB,
	//	d->moveCmdArgLR,
	//};
	//CommandParams params = GMPhysicsWorld::makeCommand(CMD_MOVE, args, 3);
	//pw->sendCommand(pw->find(this), params);
}

void GMSpriteGameObject::clearMoveArgs()
{
	//D(d);
	//d->moveCmdArgFB = GMCommandVector3(0, 0, 0);
	//d->moveCmdArgLR = GMCommandVector3(0, 0, 0);
}