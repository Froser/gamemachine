#include "stdafx.h"
#include "character.h"
#include "btBulletCollisionCommon.h"
#include "btBulletDynamicsCommon.h"
#include "BulletDynamics/Character/btKinematicCharacterController.h"
#include "utilities/assert.h"

Character::Character(const btTransform& position, btScalar radius, btScalar height, btScalar stepHeight)
	: m_radius(radius)
	, m_height(height)
	, m_stepHeight(stepHeight)
{
	setTransform(position);
	m_camera.setPosition(position.getOrigin().x(), position.getOrigin().y(), position.getOrigin().z());
}

btCollisionShape* Character::createCollisionShape()
{
	return new btCapsuleShape(m_radius, m_height);
}

void Character::appendObjectToWorld(btDynamicsWorld* world)
{
	btPairCachingGhostObject* ghostObj = new btPairCachingGhostObject();

	ghostObj->setWorldTransform(dataRef().getTransform());
	ghostObj->setCollisionShape(getCollisionShape());
	ghostObj->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
	setCollisionObject(ghostObj);

	m_controller = new btKinematicCharacterController(ghostObj, static_cast<btConvexShape*>(getCollisionShape()), m_stepHeight);
	m_controller->setGravity(world->getGravity());

	world->addCollisionObject(ghostObj,
		btBroadphaseProxy::CharacterFilter,
		btBroadphaseProxy::StaticFilter | btBroadphaseProxy::AllFilter);
	world->addAction(m_controller);
}

void Character::setCanFreeMove(bool freeMove)
{
	if (freeMove)
		m_camera.setType(Camera::FreeCamera);
	else
		m_camera.setType(Camera::PhysicalCamera);
}

void Character::simulateCamera()
{
	btTransform& transform = getCollisionObject()->getWorldTransform();
	btVector3 pos = transform.getOrigin();
	m_camera.setPosition(pos.getX(), pos.getY(), pos.getZ());
}

Camera& Character::getCamera()
{
	return m_camera;
}

void Character::moveFront(GMfloat distance)
{
	m_camera.moveFront(distance);
	move();
}

void Character::moveRight(GMfloat distance)
{
	m_camera.moveRight(distance);
	move();
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

void Character::move()
{
	btCollisionObject* colObj = getCollisionObject();
	btTransform transform;
	CameraLookAt lookAt = m_camera.getCameraLookAt();
	transform.setIdentity();
	transform.setOrigin(btVector3(lookAt.position_x, lookAt.position_y, lookAt.position_z));
	colObj->setWorldTransform(transform);
}