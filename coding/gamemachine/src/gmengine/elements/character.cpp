#include "stdafx.h"
#include "character.h"
#include "utilities/assert.h"
#include "gmengine/elements/gameworld.h"
#include "gmengine/controllers/gameloop.h"

Character::Character(GMfloat radius)
	: m_radius(radius)
	, m_jumpSpeed(vmath::vec3(0, 10, 0))
	, m_freeMove(true)
	, m_moveSpeed(10)
	, m_frustum(75, 1.333f, 0.1f, 3200)
	, m_moveDirection(0)
{
	memset(&m_state, 0, sizeof(m_state));
	memset(&m_eyeOffset, 0, sizeof(m_eyeOffset));
	memset(&m_walkDirectionFB, 0, sizeof(m_walkDirectionFB));
	memset(&m_walkDirectionLR, 0, sizeof(m_walkDirectionLR));
	memset(&m_moveRate, 0, sizeof(m_moveRate));
	m_state.pitchLimitRad = HALF_PI - RAD(3);
	setMoveSpeed(1);
}

void Character::onAppendingObjectToWorld()
{
	applyWalkDirection();
}

GMfloat Character::calcMoveDistance(GMfloat rate)
{
	GMfloat fps = getWorld()->getGraphicEngine()->getGraphicSettings()->fps;
	return m_moveSpeed;
}

void Character::moveForwardOrBackward(bool forward)
{
	GMfloat distance = (forward ? 1 : -1 ) * calcMoveDistance(forward ? m_moveRate.getMoveRate(MD_FORWARD) : m_moveRate.getMoveRate(MD_BACKWARD));
	GMfloat l = m_freeMove ? distance * std::cos(m_state.pitch) : distance;
	m_walkDirectionFB[0] = l * std::sin(m_state.yaw);
	m_walkDirectionFB[1] = m_freeMove ? distance * std::sin(m_state.pitch) : 0;
	m_walkDirectionFB[2] = -l * std::cos(m_state.yaw);
	applyWalkDirection();
}

void Character::moveLeftOrRight(bool left)
{
	GMfloat distance = (left ? -1 : 1) * calcMoveDistance(left ? m_moveRate.getMoveRate(MD_LEFT) : m_moveRate.getMoveRate(MD_RIGHT));
	m_walkDirectionLR[0] = distance * std::cos(m_state.yaw);
	m_walkDirectionLR[1] = 0;
	m_walkDirectionLR[2] = distance * std::sin(m_state.yaw);
	applyWalkDirection();
}

void Character::setJumpSpeed(const vmath::vec3& jumpSpeed)
{
	m_jumpSpeed = jumpSpeed;
}

void Character::setCanFreeMove(bool freeMove)
{
	m_freeMove = freeMove;

	/*
	if (m_freeMove)
		m_controller->setGravity(btVector3(0, 0, 0));
	else
		m_controller->setGravity(m_dynamicWorld->getGravity());
		*/
}

void Character::setMoveSpeed(GMfloat moveSpeed)
{
	m_moveSpeed = moveSpeed;
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

void Character::setEyeOffset(GMfloat* offset)
{
	memcpy(&m_eyeOffset, offset, sizeof(m_eyeOffset));
}

void Character::getReadyForRender(DrawingList& list)
{
}

void Character::simulation()
{
	// forward has priority
	if (m_moveDirection & MD_FORWARD)
	{
		moveForwardOrBackward(true);
	}
	else if (m_moveDirection & MD_BACKWARD)
	{
		moveForwardOrBackward(false);
	}
	else
	{
		memset(m_walkDirectionFB, 0, sizeof(m_walkDirectionFB));
		applyWalkDirection();
	}

	if (m_moveDirection & MD_LEFT)
	{
		moveLeftOrRight(true);
	}
	else if (m_moveDirection & MD_RIGHT)
	{
		moveLeftOrRight(false);
	}
	else
	{
		memset(m_walkDirectionLR, 0, sizeof(m_walkDirectionLR));
		applyWalkDirection();
	}

	if (m_moveDirection & MD_JUMP)
	{
		/*
		if (m_controller->canJump())
		{
			m_controller->jump(TO_BT(m_jumpSpeed));
		}
		*/
	}
}

void Character::updateCamera()
{
	update();
	Camera::calcCameraLookAt(m_state, &m_lookAt);
}

void Character::update()
{
	CollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
	{
		m_state.position = c->motions.translation + m_eyeOffset;
	}
	else
	{
		gm_error("cannot found character in physics world");
	}
}

void Character::applyWalkDirection()
{
	CollisionObject* c = getWorld()->physicsWorld()->find(this);
	if (c)
	{
		c->motions.velocity = vmath::vec3(
			m_walkDirectionFB[0] + m_walkDirectionLR[0],
			m_walkDirectionFB[1] + m_walkDirectionLR[1],
			m_walkDirectionFB[2] + m_walkDirectionLR[2]
		);
	}
	else
	{
		gm_error("cannot found character in physics world");
	}
}

CameraLookAt& Character::getLookAt()
{
	return m_lookAt;
}

Frustum& Character::getFrustum()
{
	return m_frustum;
}