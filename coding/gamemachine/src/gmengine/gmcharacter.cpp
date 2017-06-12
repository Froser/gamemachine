#include "stdafx.h"
#include "gmcharacter.h"
#include "gmengine/gmgameworld.h"

GMCharacter::GMCharacter(GMfloat radius)
	: GMGameObject(nullptr)
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

void GMCharacter::onAppendingObjectToWorld()
{
	sendMoveCommand();
}

void GMCharacter::moveForwardOrBackward(bool forward)
{
	D(d);
	GMfloat moveRate = forward ? d->moveRate.getMoveRate(MD_FORWARD) : d->moveRate.getMoveRate(MD_BACKWARD);
	d->moveCmdArgFB = GMCommandVector3(forward, moveRate, USELESS_PARAM);
}

void GMCharacter::moveLeftOrRight(bool left)
{
	D(d);
	GMfloat moveRate = left ? d->moveRate.getMoveRate(MD_LEFT) : d->moveRate.getMoveRate(MD_RIGHT);
	d->moveCmdArgLR = GMCommandVector3(left, moveRate, USELESS_PARAM);
}

void GMCharacter::setJumpSpeed(const linear_math::Vector3& jumpSpeed)
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.jumpSpeed = jumpSpeed;
}

void GMCharacter::setMoveSpeed(GMfloat moveSpeed)
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		c->motions.moveSpeed = moveSpeed;
}

const PositionState& GMCharacter::getPositionState()
{
	D(d);
	return d->state;
}

// rate表示移动的速度，如果来自键盘，那么应该为1，如果来自手柄，应该是手柄的delta/delta最大值
void GMCharacter::action(MoveAction md, MoveRate rate)
{
	D(d);
	d->moveDirection = md;
	d->moveRate = rate;
}

void GMCharacter::lookRight(GMfloat degree)
{
	D(d);
	d->state.yaw += RAD(degree);
}

void GMCharacter::lookUp(GMfloat degree)
{
	D(d);
	d->state.pitch += RAD(degree);
	if (d->state.pitch > d->state.pitchLimitRad)
		d->state.pitch = d->state.pitchLimitRad;
	else if (d->state.pitch < -d->state.pitchLimitRad)
		d->state.pitch = -d->state.pitchLimitRad;
}

void GMCharacter::setPitchLimitDegree(GMfloat deg)
{
	D(d);
	d->state.pitchLimitRad = HALF_PI - RAD(deg);
}

void GMCharacter::simulation()
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

	d->moveDirection = MD_NONE;
}

void GMCharacter::updateCamera()
{
	D(d);
	update();
	Camera::calcCameraLookAt(d->state, d->lookAt);
}

void GMCharacter::update()
{
	D(d);
	GMCollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		d->state.position = c->motions.translation;
	else
		gm_error(_L("cannot found character in physics world"));
}

void GMCharacter::sendMoveCommand()
{
	D(d);
	GMPhysicsWorld* pw = getWorld()->physicsWorld();
	GMCommandVector3 args[] = {
		GMCommandVector3(d->state.pitch, d->state.yaw, USELESS_PARAM), d->moveCmdArgFB, d->moveCmdArgLR,
	};
	pw->sendCommand(pw->find(this), GMPhysicsWorld::makeCommand(CMD_MOVE, args, 3));
}

void GMCharacter::clearMoveArgs()
{
	D(d);
	d->moveCmdArgFB = GMCommandVector3(0, 0, 0);
	d->moveCmdArgLR = GMCommandVector3(0, 0, 0);
}

CameraLookAt& GMCharacter::getLookAt()
{
	D(d);
	return d->lookAt;
}

Frustum& GMCharacter::getFrustum()
{
	D(d);
	return d->frustum;
}