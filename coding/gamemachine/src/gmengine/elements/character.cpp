#include "stdafx.h"
#include "character.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "utilities/assert.h"
#include "gmengine/elements/gameworld.h"

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
{
	setTransform(position);
	memset(&m_state, 0, sizeof(m_state));
	m_state.positionX = position.getOrigin().x();
	m_state.positionY = position.getOrigin().y();
	m_state.positionZ = position.getOrigin().z();
	m_state.lookUpLimitRad = HALF_PI - RAD(3);
}

btCollisionShape* Character::createCollisionShape()
{
	return new btCapsuleShape(m_radius, m_height);
}

void Character::appendObjectToWorld(btDynamicsWorld* world)
{
	D(d);
	btPairCachingGhostObject* ghostObj = new btPairCachingGhostObject();

	ghostObj->setWorldTransform(d.transform);
	ghostObj->setCollisionShape(getCollisionShape());
	ghostObj->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	setCollisionObject(ghostObj);

	m_controller.reset(new btKinematicCharacterController(ghostObj, static_cast<btConvexShape*>(getCollisionShape()), m_stepHeight));
	if (m_freeMove)
		m_controller->setGravity(btVector3(0, 0, 0));
	else
		m_controller->setGravity(world->getGravity());
	m_controller->setFallSpeed(m_fallSpeed);

	world->addCollisionObject(ghostObj,
		btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::AllFilter);
	world->addAction(m_controller);
	m_dynamicWorld = world;
}

GMfloat Character::calcMoveDistance()
{
	return m_moveSpeed / getWorld()->getGraphicEngine()->getGraphicSettings()->fps;
}

void Character::moveForwardOrBackward(bool forward)
{
	GMfloat distance = (forward ? 1 : -1 ) * calcMoveDistance();
	if (m_freeMove)
		m_state.positionY += distance * std::sin(m_state.lookUpRad);

	GMfloat l = m_freeMove ? distance * std::cos(m_state.lookUpRad) : distance;
	m_state.positionX += l * std::sin(m_state.lookAtRad);
	m_state.positionZ -= l * std::cos(m_state.lookAtRad);
}

void Character::moveLeftOrRight(bool right)
{
	GMfloat distance = (right ? 1 : -1) * calcMoveDistance();
	m_state.positionX += distance * std::cos(m_state.lookAtRad);
	m_state.positionZ += distance * std::sin(m_state.lookAtRad);
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

void Character::simulateCamera()
{
	btTransform& trans = getCollisionObject()->getWorldTransform();
	m_state.positionX = trans.getOrigin().x();
	m_state.positionY = trans.getOrigin().y();
	m_state.positionZ = trans.getOrigin().z();
}

void Character::setMoveSpeed(GMfloat moveSpeed)
{
	m_moveSpeed = moveSpeed;
}

const PositionState& Character::getPositionState()
{
	return m_state;
}

void Character::moveForward()
{
	moveForwardOrBackward(true);
	moveCollisionObject();
}

void Character::moveBackward()
{
	moveForwardOrBackward(false);
	moveCollisionObject();
}

void Character::moveRight()
{
	moveLeftOrRight(true);
	moveCollisionObject();
}

void Character::moveLeft()
{
	moveLeftOrRight(false);
	moveCollisionObject();
}

void Character::moveCollisionObject()
{
	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(m_state.positionX, m_state.positionY, m_state.positionZ));
	getCollisionObject()->setWorldTransform(trans);
}

void Character::lookRight(GMfloat degree)
{
	m_state.lookAtRad += RAD(degree);
}

void Character::lookUp(GMfloat degree)
{
	m_state.lookUpRad += RAD(degree);
	if (m_state.lookUpRad > m_state.lookUpLimitRad)
		m_state.lookUpRad = m_state.lookUpLimitRad;
	else if (m_state.lookUpRad < -m_state.lookUpLimitRad)
		m_state.lookUpRad = -m_state.lookUpLimitRad;
}

void Character::setLookUpLimitDegree(GMfloat deg)
{
	m_state.lookUpLimitRad = HALF_PI - RAD(deg);
}

void Character::jump()
{
	if (m_controller->canJump())
		m_controller->jump(m_jumpSpeed);
}

bool Character::isJumping()
{
	return !m_controller->canJump();
}

void Character::setEyeOffset(GMfloat* offset)
{
	memcpy(&m_eyeOffset, offset, sizeof(m_eyeOffset));
}

void Character::applyEyeOffset(CameraLookAt& lookAt)
{
	lookAt.position_x += m_eyeOffset[0];
	lookAt.position_y += m_eyeOffset[1];
	lookAt.position_z += m_eyeOffset[2];
}

void Character::getReadyForRender(DrawingList& list)
{
}