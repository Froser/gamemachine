#include "stdafx.h"
#include "gmgameworld.h"
#include "gmspritegameobject.h"
#include "gmphysics/gmphysicsworld.h"

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius)
	: GMGameObject(nullptr)
{
	D(d);
	d->radius = radius;
	d->moveDirection = MC_NONE;
	d->state = { 0 };
	d->moveRate.clear();
	clearMoveArgs();
	d->pitchLimitRadius = HALF_PI - RAD(3);
}


void GMSpriteGameObject::onAppendingObjectToWorld()
{
	sendMoveCommand();
}

void GMSpriteGameObject::moveForwardOrBackward(bool forward)
{
	D(d);
	GMfloat moveRate = forward ? d->moveRate.getMoveRate(MC_FORWARD) : d->moveRate.getMoveRate(MC_BACKWARD);
	d->moveCmdArgFB = GMCommandVector3(forward, moveRate, USELESS_PARAM);
}

void GMSpriteGameObject::moveLeftOrRight(bool left)
{
	D(d);
	GMfloat moveRate = left ? d->moveRate.getMoveRate(MC_LEFT) : d->moveRate.getMoveRate(MC_RIGHT);
	d->moveCmdArgLR = GMCommandVector3(left, moveRate, USELESS_PARAM);
}

void GMSpriteGameObject::setJumpSpeed(const linear_math::Vector3& jumpSpeed)
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.jumpSpeed = jumpSpeed;
}

void GMSpriteGameObject::setMoveSpeed(GMfloat moveSpeed)
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.moveSpeed = moveSpeed;
}

const PositionState& GMSpriteGameObject::getPositionState()
{
	D(d);
	return d->state;
}

// rate表示移动的速度，如果来自键盘，那么应该为1，如果来自手柄，应该是手柄的delta/delta最大值
void GMSpriteGameObject::action(GMMovement movement, const GMMoveRate& rate)
{
	D(d);
	d->moveDirection = movement;
	d->moveRate = rate;
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

	// forward has priority
	bool moved = false;
	if (d->moveDirection & MC_FORWARD)
	{
		moveForwardOrBackward(true);
		moved = true;
	}
	else if (d->moveDirection & MC_BACKWARD)
	{
		moveForwardOrBackward(false);
		moved = true;
	}

	if (d->moveDirection & MC_LEFT)
	{
		moveLeftOrRight(true);
		moved = true;
	}
	else if (d->moveDirection & MC_RIGHT)
	{
		moveLeftOrRight(false);
		moved = true;
	}

	if (moved)
		sendMoveCommand();

	if (d->moveDirection & MC_JUMP)
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

	d->moveDirection = MC_NONE;
}

void GMSpriteGameObject::updateAfterSimulate()
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		d->state.position = c->motions.translation;
	else
		gm_error(_L("cannot found character in physics world"));
}

void GMSpriteGameObject::sendMoveCommand()
{
	D(d);
	GMPhysicsWorld* pw = getWorld()->physicsWorld();
	GMCommandVector3 args[] = {
		GMCommandVector3(d->state.pitch, d->state.yaw, USELESS_PARAM),
		d->moveCmdArgFB,
		d->moveCmdArgLR,
	};
	CommandParams params = GMPhysicsWorld::makeCommand(CMD_MOVE, args, 3);
	pw->sendCommand(pw->find(this), params);
}

void GMSpriteGameObject::clearMoveArgs()
{
	D(d);
	d->moveCmdArgFB = GMCommandVector3(0, 0, 0);
	d->moveCmdArgLR = GMCommandVector3(0, 0, 0);
}