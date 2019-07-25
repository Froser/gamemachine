#include "stdafx.h"
#include <gmgameworld.h>
#include "gmspritegameobject.h"
#include "gmphysics/gmphysicsworld.h"

BEGIN_NS

GM_PRIVATE_OBJECT_ALIGNED(GMSpriteGameObject)
{
	GMfloat radius;
	GMfloat limitPitch;
	AlignedVector<GMSpriteMovement> movements;
	GMVec3 moveSpeed = Zero<GMVec3>();
	GMVec3 jumpSpeed = Zero<GMVec3>();
	GMCamera camera;
	GMCameraUtility cameraUtility;
};

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius, const GMVec3& position)
{
	GM_CREATE_DATA();
	D(d);
	d->radius = radius;
	d->camera.lookAt(GMCameraLookAt(GMVec3(0, 0, -1), position));
	d->limitPitch = Radians(85.f);
	d->cameraUtility.setCamera(&d->camera);
}

GMSpriteGameObject::GMSpriteGameObject(GMfloat radius, const GMCamera& camera)
{
	GM_CREATE_DATA();
	D(d);
	d->radius = radius;
	d->camera = camera;
	d->limitPitch = Radians(85.f);
	d->cameraUtility.setCamera(&d->camera);
}

void GMSpriteGameObject::action(GMMovement movement, const GMVec3& direction, const GMVec3& rate)
{
	D(d);
	const GMVec3* speed;
	if (movement == GMMovement::Move)
		speed = &d->moveSpeed;
	else if (movement == GMMovement::Jump)
		speed = &d->jumpSpeed;
	else
		GM_ASSERT(false);

	GMSpriteMovement subMovement(direction, rate, *speed, movement);
	d->movements.push_back(subMovement);
}

void GMSpriteGameObject::look(GMfloat pitch, GMfloat yaw)
{
	D(d);
	d->cameraUtility.update(yaw, pitch);
}

const GMCamera& GMSpriteGameObject::getCamera() GM_NOEXCEPT
{
	D(d);
	return d->camera;
}

void GMSpriteGameObject::setPosition(const GMVec3& position)
{
	D(d);
	auto lookAt = d->camera.getLookAt();
	lookAt.position = position;
	d->camera.lookAt(lookAt);
}


void GMSpriteGameObject::setJumpSpeed(const GMVec3& speed)
{
	D(d);
	d->jumpSpeed = speed;
}

void GMSpriteGameObject::setMoveSpeed(const GMVec3& speed)
{
	D(d);
	d->moveSpeed = speed;
}

void GMSpriteGameObject::update(GMDuration dt)
{
	D(d);
	Base::update(dt);
	GMVec3 direction(0), rate(0), moveSpeed(0), jumpSpeed(0);
	bool moved = false, jumped = false, rateAssigned = false;
	for (auto& movement : d->movements)
	{
		if (movement.movement == GMMovement::Move)
		{
			moveSpeed = movement.speed;
			moved = true;
		}
		else if (movement.movement == GMMovement::Jump)
		{
			jumpSpeed = movement.speed;
			jumped = true;
		}
		direction += movement.moveDirection;
		if (!rateAssigned)
		{
			rate = movement.moveRate;
			rateAssigned = true;
		}
	}
	direction = FastNormalize(direction);

	if (auto phyObj = getPhysicsObject())
	{
		const GMVec3& lookDirection = d->camera.getLookAt().lookDirection;
		if (moved)
		{
			GMPhysicsMoveArgs args(lookDirection, direction, moveSpeed, rate);
			GMPhysicsWorld* world = getWorld()->getPhysicsWorld();
			if (world)
				world->applyMove(getPhysicsObject(), args);
		}

		if (jumped)
		{
			GMPhysicsMoveArgs args(lookDirection, direction, jumpSpeed, rate);
			GMPhysicsWorld* world = getWorld()->getPhysicsWorld();
			if (world)
				world->applyJump(getPhysicsObject(), args);
		}

		GMFloat4 f4_position;
		GetTranslationFromMatrix(phyObj->getMotionStates().transform, f4_position);
		d->camera.getLookAt().position.setFloat4(f4_position);
	}
	d->movements.clear();
}

END_NS