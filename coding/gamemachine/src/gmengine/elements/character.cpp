#include "stdafx.h"
#include "character.h"
#include "utilities/assert.h"
#include "gmengine/elements/gameworld.h"

Character::Character(GMfloat radius)
	: GameObject(nullptr)
	, m_radius(radius)
	, m_frustum(75, 1.333f, 0.1f, 3200)
	, m_moveDirection(0)
{
	memset(&m_state, 0, sizeof(m_state));
	memset(&m_moveRate, 0, sizeof(m_moveRate));
	clearMoveArgs();

	m_state.pitchLimitRad = HALF_PI - RAD(3);
}

void Character::onAppendingObjectToWorld()
{
	sendMoveCommand();
}

void Character::moveForwardOrBackward(bool forward)
{
	GMfloat moveRate = forward ? m_moveRate.getMoveRate(MD_FORWARD) : m_moveRate.getMoveRate(MD_BACKWARD);
	m_moveCmdArgFB = vmath::vec3(forward, moveRate, USELESS_PARAM);
}

void Character::moveLeftOrRight(bool left)
{
	GMfloat moveRate = left ? m_moveRate.getMoveRate(MD_LEFT) : m_moveRate.getMoveRate(MD_RIGHT);
	m_moveCmdArgLR = vmath::vec3(left, moveRate, USELESS_PARAM);
}

void Character::setJumpSpeed(const vmath::vec3& jumpSpeed)
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
	return m_state;
}

// rate表示移动的速度，如果来自键盘，那么应该为1，如果来自手柄，应该是手柄的delta/delta最大值
void Character::action(MoveAction md, MoveRate rate)
{
	m_moveDirection = md;
	m_moveRate = rate;
}

void Character::lookRight(GMfloat degree)
{
	m_state.yaw += RAD(degree);
}

void Character::lookUp(GMfloat degree)
{
	m_state.pitch += RAD(degree);
	if (m_state.pitch > m_state.pitchLimitRad)
		m_state.pitch = m_state.pitchLimitRad;
	else if (m_state.pitch < -m_state.pitchLimitRad)
		m_state.pitch = -m_state.pitchLimitRad;
}

void Character::setPitchLimitDegree(GMfloat deg)
{
	m_state.pitchLimitRad = HALF_PI - RAD(deg);
}

void Character::getReadyForRender(DrawingList& list)
{
}

void Character::simulation()
{
	clearMoveArgs();

	// forward has priority
	bool moved = false;
	if (m_moveDirection & MD_FORWARD)
	{
		moveForwardOrBackward(true);
		moved = true;
	}
	else if (m_moveDirection & MD_BACKWARD)
	{
		moveForwardOrBackward(false);
		moved = true;
	}

	if (m_moveDirection & MD_LEFT)
	{
		moveLeftOrRight(true);
		moved = true;
	}
	else if (m_moveDirection & MD_RIGHT)
	{
		moveLeftOrRight(false);
		moved = true;
	}

	if (moved)
		sendMoveCommand();

	if (m_moveDirection & MD_JUMP)
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

	m_moveDirection = MD_NONE;
}

void Character::updateCamera()
{
	update();
	Camera::calcCameraLookAt(m_state, m_lookAt);
}

void Character::update()
{
	CollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
		m_state.position = c->motions.translation;
	else
		gm_error("cannot found character in physics world");
}

void Character::sendMoveCommand()
{
	D(d);
	PhysicsWorld* pw = getWorld()->physicsWorld();
	vmath::vec3 args[] = {
		vmath::vec3(m_state.pitch, m_state.yaw, USELESS_PARAM), m_moveCmdArgFB, m_moveCmdArgLR,
	};
	pw->sendCommand(pw->find(this), PhysicsWorld::makeCommand(CMD_MOVE, args, 3));
}

void Character::clearMoveArgs()
{
	m_moveCmdArgFB = vmath::vec3(0);
	m_moveCmdArgLR = vmath::vec3(0);
}

CameraLookAt& Character::getLookAt()
{
	return m_lookAt;
}

Frustum& Character::getFrustum()
{
	return m_frustum;
}