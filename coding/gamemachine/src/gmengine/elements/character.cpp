#include "stdafx.h"
#include "character.h"
#include "gmengine/elements/gameworld.h"

Character::Character(GMfloat radius)
	: GameObject(nullptr)
{
	D(d);
	d->radius = radius;
	d->frustum.initFrustum(75.f, 1.333f, .1f, 3200);
	d->moveDirection = MD_NONE;

	memset(&d->state, 0, sizeof(d->state));
	memset(&d->moveRate, 0, sizeof(d->moveRate));
	clearMoveArgs();

	d->state.pitchLimitRad = HALF_PI - RAD(3);
}

void Character::onAppendingObjectToWorld()
{
	sendMoveCommand();
}

void Character::moveForwardOrBackward(bool forward)
{
	D(d);
	GMfloat moveRate = forward ? d->moveRate.getMoveRate(MD_FORWARD) : d->moveRate.getMoveRate(MD_BACKWARD);
	d->moveCmdArgFB = CommandVector3(forward, moveRate, USELESS_PARAM);
}

void Character::moveLeftOrRight(bool left)
{
	D(d);
	GMfloat moveRate = left ? d->moveRate.getMoveRate(MD_LEFT) : d->moveRate.getMoveRate(MD_RIGHT);
	d->moveCmdArgLR = CommandVector3(left, moveRate, USELESS_PARAM);
}

void Character::setJumpSpeed(const linear_math::Vector3& jumpSpeed)
{
	D(d);
	CollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.jumpSpeed = jumpSpeed;
}

void Character::setMoveSpeed(GMfloat moveSpeed)
{
	D(d);
	CollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.moveSpeed = moveSpeed;
}

const PositionState& Character::getPositionState()
{
	D(d);
	return d->state;
}

// rate表示移动的速度，如果来自键盘，那么应该为1，如果来自手柄，应该是手柄的delta/delta最大值
void Character::action(MoveAction md, MoveRate rate)
{
	D(d);
	d->moveDirection = md;
	d->moveRate = rate;
}

void Character::lookRight(GMfloat degree)
{
	D(d);
	d->state.yaw += RAD(degree);
}

void Character::lookUp(GMfloat degree)
{
	D(d);
	d->state.pitch += RAD(degree);
	if (d->state.pitch > d->state.pitchLimitRad)
		d->state.pitch = d->state.pitchLimitRad;
	else if (d->state.pitch < -d->state.pitchLimitRad)
		d->state.pitch = -d->state.pitchLimitRad;
}

void Character::setPitchLimitDegree(GMfloat deg)
{
	D(d);
	d->state.pitchLimitRad = HALF_PI - RAD(deg);
}

void Character::simulation()
{
	D(d);
	clearMoveArgs();

	// forward has priority
	bool moved = false;
	if (d->moveDirection & MD_FORWARD)
	{
		moveForwardOrBackward(true);
		moved = true;
	}
	else if (d->moveDirection & MD_BACKWARD)
	{
		moveForwardOrBackward(false);
		moved = true;
	}

	if (d->moveDirection & MD_LEFT)
	{
		moveLeftOrRight(true);
		moved = true;
	}
	else if (d->moveDirection & MD_RIGHT)
	{
		moveLeftOrRight(false);
		moved = true;
	}

	if (moved)
		sendMoveCommand();

	if (d->moveDirection & MD_JUMP)
	{
		PhysicsWorld* world = getWorld()->physicsWorld();
		CollisionObject* c = getWorld()->physicsWorld()->find(this);
		if (c)
		{
			CommandParams cmdParams = PhysicsWorld::makeCommand(CMD_JUMP, nullptr, 0);
			world->sendCommand(c, cmdParams);
		}
		else
		{
			gm_error("cannot found character in physics world");
		}
	}

	d->moveDirection = MD_NONE;
}

void Character::updateCamera()
{
	D(d);
	update();
	Camera::calcCameraLookAt(d->state, d->lookAt);
}

void Character::update()
{
	D(d);
	CollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		d->state.position = c->motions.translation;
	else
		gm_error("cannot found character in physics world");
}

void Character::sendMoveCommand()
{
	D(d);
	PhysicsWorld* pw = getWorld()->physicsWorld();
	CommandVector3 args[] = {
		CommandVector3(d->state.pitch, d->state.yaw, USELESS_PARAM), d->moveCmdArgFB, d->moveCmdArgLR,
	};
	pw->sendCommand(pw->find(this), PhysicsWorld::makeCommand(CMD_MOVE, args, 3));
}

void Character::clearMoveArgs()
{
	D(d);
	d->moveCmdArgFB = CommandVector3(0, 0, 0);
	d->moveCmdArgLR = CommandVector3(0, 0, 0);
}

CameraLookAt& Character::getLookAt()
{
	D(d);
	return d->lookAt;
}

Frustum& Character::getFrustum()
{
	D(d);
	return d->frustum;
}