#include "stdafx.h"
#include "character.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "utilities/assert.h"
#include "gmengine/elements/gameworld.h"
#include "../flow/gameloop.h"

Character::Character(const btTransform& position, btScalar radius, btScalar height, btScalar stepHeight)
	: m_radius(radius)
	, m_height(height)
	, m_stepHeight(stepHeight)
	, m_controller(nullptr)
	, m_jumpSpeed(btVector3(0, 10, 0))
	, m_fallSpeed(10)
	, m_freeMove(true)
	, m_dynamicWorld(nullptr)
	, m_moveSpeed(10)
	, m_frustum(75, 1.333f, 0.1, 100)
	, m_moveDirection(0)
{
	setTransform(position);
	memset(&m_state, 0, sizeof(m_state));
	memset(&m_eyeOffset, 0, sizeof(m_eyeOffset));
	m_state.positionX = position.getOrigin().x();
	m_state.positionY = position.getOrigin().y();
	m_state.positionZ = position.getOrigin().z();
	m_state.pitchLimitRad = HALF_PI - RAD(3);
	setMoveSpeed(1);
}

btCollisionShape* Character::createCollisionShape()
{
	return new btCapsuleShape(m_radius, m_height);
}

void Character::appendThisObjectToWorld(btDynamicsWorld* world)
{
	D(d);

	// gravity
	btPairCachingGhostObject* collisionObject = static_cast<btPairCachingGhostObject*>(d.collisionObject);
	m_controller.reset(new btKinematicCharacterController(collisionObject, static_cast<btConvexShape*>(d.collisionShape.get()), m_stepHeight));
	if (m_freeMove)
		m_controller->setGravity(btVector3(0, 0, 0));
	else
		m_controller->setGravity(world->getGravity());
	m_controller->setFallSpeed(m_fallSpeed);

	world->addCollisionObject(d.collisionObject,
		btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::AllFilter);
	world->addAction(m_controller);
	m_dynamicWorld = world;
}

btCollisionObject* Character::createCollisionObject()
{
	D(d);
	btPairCachingGhostObject* ghostObj = new btPairCachingGhostObject();
	ghostObj->setWorldTransform(d.transform);
	ghostObj->setCollisionShape(d.collisionShape);
	ghostObj->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	return ghostObj;
}

GMfloat Character::calcMoveDistance()
{
	GMfloat elapsed = GameLoop::getInstance()->getElapsedAfterLastFrame();
	GMfloat fps = getWorld()->getGraphicEngine()->getGraphicSettings()->fps;
	GMfloat skipFrame = elapsed / (1.0f / fps);
	if (skipFrame > 1)
		return m_moveSpeed * skipFrame / fps;
	else
		return m_moveSpeed / fps;
}

void Character::moveForwardOrBackward(bool forward)
{
	GMfloat distance = (forward ? 1 : -1 ) * calcMoveDistance();
	if (m_freeMove)
		m_state.positionY += distance * std::sin(m_state.pitch);

	GMfloat l = m_freeMove ? distance * std::cos(m_state.pitch) : distance;
	m_walkDirectionFB[0] = l * std::sin(m_state.yaw);
	m_walkDirectionFB[1] = m_freeMove ? distance * std::sin(m_state.pitch) : 0;
	m_walkDirectionFB[2] = -l * std::cos(m_state.yaw);
	applyWalkDirection();
}

void Character::moveLeftOrRight(bool left)
{
	GMfloat distance = (left ? -1 : 1) * calcMoveDistance();
	m_walkDirectionLR[0] = distance * std::cos(m_state.yaw);
	m_walkDirectionLR[1] = 0;
	m_walkDirectionLR[2] = distance * std::sin(m_state.yaw);
	applyWalkDirection();
}

void Character::setJumpSpeed(const btVector3& jumpSpeed)
{
	m_jumpSpeed = jumpSpeed;
}

void Character::setFallSpeed(GMfloat speed)
{
	m_fallSpeed = speed;
	if (m_controller)
		m_controller->setFallSpeed(m_fallSpeed);
}

void Character::setCanFreeMove(bool freeMove)
{
	m_freeMove = freeMove;

	if (m_dynamicWorld)
	{
		if (m_freeMove)
			m_controller->setGravity(btVector3(0, 0, 0));
		else
			m_controller->setGravity(m_dynamicWorld->getGravity());
	}
}

void Character::setMoveSpeed(GMfloat moveSpeed)
{
	m_moveSpeed = moveSpeed;
}

const PositionState& Character::getPositionState()
{
	return m_state;
}

void Character::action(MoveDirection md)
{
	m_moveDirection = md;
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
		if (m_controller->canJump())
			m_controller->jump(m_jumpSpeed);
	}
}

void Character::updateCamera()
{
	update();
	Camera::calcCameraLookAt(m_state, &m_lookAt);
}

void Character::update()
{
	btTransform trans = m_controller->getGhostObject()->getWorldTransform();
	m_state.positionX = trans.getOrigin().x() + m_eyeOffset[0];
	m_state.positionY = trans.getOrigin().y() + m_eyeOffset[1];
	m_state.positionZ = trans.getOrigin().z() + m_eyeOffset[2];
}

void Character::applyWalkDirection()
{
	m_controller->setWalkDirection(btVector3(
		m_walkDirectionFB[0] + m_walkDirectionLR[0], 
		m_walkDirectionFB[1] + m_walkDirectionLR[1],
		m_walkDirectionFB[2] + m_walkDirectionLR[2])
	);
}

CameraLookAt& Character::getLookAt()
{
	return m_lookAt;
}

Frustum& Character::getFrustum()
{
	return m_frustum;
}