#include "stdafx.h"
#include "gmbspmove.h"
#include "extensions/bsp/render/gmbspgameworld.h"
#include <extensions/bsp/gmbspphysicsworld.h>
#include "gmphysics/gmphysicsobject.h"
#include "foundation/gamemachine.h"
#include "foundation/gmprofile.h"

static const GMfloat OVERCLIP = 1.f;
static const GMfloat CLIP_IGNORE = .2f;

GMBSPMove::GMBSPMove(GMBSPPhysicsWorld* world, GMPhysicsObject* obj)
{
	D(d);
	d->inited = false;
	d->world = world;
	d->object = obj;
	d->trace = &world->physicsData().trace;
}

void GMBSPMove::move(GMDuration dt)
{
	D(d);
	generateMovement();
	groundTrace();
	processCommand();
	if (d->movementState.walking)
		walkMove(dt);
	else
		airMove(dt);
}

void GMBSPMove::processCommand()
{
	D(d);
	if (d->action.move.moved)
	{
		processMove();
		d->action.move.moved = false;
	}
	else
	{
		//TODO 没有在move的时候，可以考虑摩擦使速度减小
		//这里我们先清空速度
		if (!d->movementState.freefall)
			d->initialVelocity = Zero<GMVec3>();
	}

	if (d->action.jump.jumped)
	{
		processJump();
		d->action.jump.jumped = false;
	}
}

void GMBSPMove::processMove()
{
	D(d);
	// 空中不允许改变运动状态
	if (d->movementState.freefall)
		return;

	GMPhysicsMoveArgs moveArgs = d->action.move.args;
	moveArgs.lookAt.setY(0.f); // 不考虑roll，始终平视前方
	GMQuat q = GMQuat(GMVec3(0, 0, 1), moveArgs.lookAt);
	d->initialVelocity = (moveArgs.direction * moveArgs.speed * moveArgs.rate) * q;
	composeVelocityWithGravity();
}

void GMBSPMove::processJump()
{
	D(d);
	if (!d->movementState.freefall)
	{
		// 能够跳跃的场合
		GMFloat4 f4_velocity, f4_speed;
		d->movementState.velocity.loadFloat4(f4_velocity);
		d->action.jump.speed.loadFloat4(f4_speed);

		f4_velocity[0] += f4_speed[0];
		f4_velocity[1] = f4_speed[1];
		f4_velocity[2] += f4_speed[2];

		d->movementState.velocity.setFloat4(f4_velocity);
	}
}

void GMBSPMove::applyMove(const GMPhysicsMoveArgs& args)
{
	D(d);
	d->action.move.moved = true;
	d->action.move.args = args;
}

void GMBSPMove::applyJump(const GMVec3& speed)
{
	D(d);
	d->action.jump.jumped = true;
	d->action.jump.speed = speed;
}

GMfloat GMBSPMove::now()
{
	D(d);
	return GM.getRunningStates().elapsedTime;
}

void GMBSPMove::generateMovement()
{
	D(d);
	if (!d->inited)
	{
		d->movementState.velocity = decomposeVelocity(d->initialVelocity);
		d->inited = true;
	}
	else
	{
		composeVelocityWithGravity();
	}

	GMFloat4 f4_origin;
	GetTranslationFromMatrix(d->object->getMotionStates().transform, f4_origin);
	d->movementState.origin.setFloat4(f4_origin);
	d->movementState.startTime = now();
}

void GMBSPMove::composeVelocityWithGravity()
{
	// 获取当前纵向速度，并叠加上加速度
	D(d);
	GMfloat accelerationVelocity = d->movementState.velocity.getY();
	d->movementState.velocity = decomposeVelocity(d->initialVelocity);
	d->movementState.velocity.setY(accelerationVelocity);
}

GMVec3 GMBSPMove::decomposeVelocity(const GMVec3& v)
{
	D(d);
	// 将速度分解成水平面平行的分量
	GMfloat len = Length(v);
	GMFloat4 f4_v;
	v.loadFloat4(f4_v);
	f4_v[1] = 0.f;
	GMVec3 planeDir;
	planeDir.setFloat4(f4_v);

	GMVec3 normal = FastNormalize(planeDir);
	return normal * len;
}

void GMBSPMove::groundTrace()
{
	D(d);
	GMVec3 p(d->movementState.origin);
	p.setY(p.getY() - .25f);

	d->trace->trace(d->movementState.origin, p, Zero<GMVec3>(),
		gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[0],
		gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[1],
		d->movementState.groundTrace
	);

	if (d->movementState.groundTrace.fraction == 1.0)
	{
		// free fall
		d->movementState.freefall = true;
		d->movementState.walking = false;
		return;
	}

	d->movementState.freefall = false;
	d->movementState.walking = true;
}

void GMBSPMove::walkMove(GMDuration dt)
{
	D(d);
	if (d->movementState.velocity == Zero<GMVec3>())
		return;

	stepSlideMove(dt, false);
}

void GMBSPMove::airMove(GMDuration dt)
{
	stepSlideMove(dt, true);
}

void GMBSPMove::stepSlideMove(GMDuration dt, bool hasGravity)
{
	D(d);
	GMVec3 startOrigin = d->movementState.origin;
	GMVec3 startVelocity = d->movementState.velocity;
	if (!slideMove(dt, hasGravity))
	{
		synchronizeMotionStates();
		return;
	}

	BSPTraceResult t;
	GMVec3 stepUp = startOrigin;
	stepUp.setY(stepUp.getY() + gmBSPPhysicsObjectCast(d->object)->shapeProperties().stepHeight);
	d->trace->trace(
		d->movementState.origin,
		stepUp,
		Zero<GMVec3>(),
		gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[0],
		gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[1],
		t);

	if (t.allsolid)
	{
		synchronizeMotionStates();
		return;
	}

	// 从原位置stepUp
	d->movementState.origin = t.endpos;
	d->movementState.velocity = startVelocity;

	slideMove(dt, hasGravity);

	// 走下来
	GMfloat stepSize = t.endpos.getY() - startOrigin.getY();
	GMVec3 stepDown = d->movementState.origin;
	stepDown.setY(stepDown.getY() - stepSize);
	d->trace->trace(
		d->movementState.origin,
		stepDown, Zero<GMVec3>(), 
		gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[0],
		gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[1],
		t);

	if (!t.allsolid)
		d->movementState.origin = t.endpos;
	if (t.fraction < 1.f)
	{
		clipVelocity(d->movementState.velocity, t.plane.getNormal(), d->movementState.velocity, OVERCLIP);
		if (d->movementState.velocity.getY() < CLIP_IGNORE)
			d->movementState.velocity.setY(0);
	}
	synchronizeMotionStates();
}

bool GMBSPMove::slideMove(GMDuration dt, bool hasGravity)
{
	D(d);
	GMBSPPhysicsWorld::Data& wd = d->world->physicsData();
	GMVec3 velocity = d->movementState.velocity;

	GMint32 numbumps = 4, bumpcount;
	GMVec3 endVelocity, endClipVelocity;
	if (hasGravity)
	{
		endVelocity = velocity;
		endVelocity.setY(endVelocity.getY() + wd.gravity * dt);
		velocity.setY((velocity.getY() + endVelocity.getY()) * .5f);
	}

	velocity *= dt;

	AlignedVector<GMVec3> planes;
	if (!d->movementState.freefall)
		planes.push_back(d->movementState.groundTrace.plane.getNormal());

	planes.push_back(FastNormalize(velocity));

	GMfloat t = 1.0f;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		BSPTraceResult moveTrace;
		d->trace->trace(d->movementState.origin,
			d->movementState.origin + velocity * t,
			Zero<GMVec3>(),
			gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[0],
			gmBSPPhysicsObjectCast(d->object)->shapeProperties().bounding[1],
			moveTrace
		);
		
		if (moveTrace.allsolid)
		{
			// entity is completely trapped in another solid
			d->movementState.velocity.setZ(0);	// don't build up falling damage, but allow sideways acceleration
			return true;
		}
		if (moveTrace.fraction > 0)
			d->movementState.origin = moveTrace.endpos;
		if (moveTrace.fraction == 1.0f)
			break;

		t -= t * moveTrace.fraction;

		GMuint32 i;
		for (i = 0; i < planes.size(); i++)
		{
			if (Dot(moveTrace.plane.getNormal(), planes[i]) > 0.99)
				velocity += moveTrace.plane.getNormal();
		}
		if (i < planes.size())
			continue;

		planes.push_back(moveTrace.plane.getNormal());

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		GMVec3 cv; //clipVelocity
		for (i = 0; i < planes.size(); i++)
		{
			if (Dot(velocity, planes[i]) >= 0.1)
				continue; // 朝着平面前方移动，不会有交汇
			clipVelocity(velocity, planes[i], cv, OVERCLIP);
			clipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

			for (GMuint32 j = 0; j < planes.size(); j++)
			{
				if (i == j)
					continue;
				if (Dot(cv, planes[j]) >= 0.1)
					continue;

				// try clipping the move to the plane
				clipVelocity(cv, planes[j], cv, OVERCLIP);
				clipVelocity(endClipVelocity, planes[i], endClipVelocity, OVERCLIP);

				// see if it goes back into the first clip plane
				if (Dot(cv, planes[i]) >= 0)
					continue;

				// slide the original velocity along the crease
				{
					GMVec3 dir = Cross(planes[i], planes[j]);
					dir = FastNormalize(dir);
					GMfloat s = Dot(dir, velocity);
					cv = dir * s;
				}

				{
					GMVec3 dir = Cross(planes[i], planes[j]);
					dir = FastNormalize(dir);
					GMfloat s = Dot(dir, endVelocity);
					endClipVelocity = dir * s;
				}

				for (GMuint32 k = 0; k < planes.size(); k++)
				{
					if (k == i || k == j)
						continue;

					if (Dot(cv, planes[k]) >= 0.1)
						continue;

					velocity = Zero<GMVec3>();
					return true;
				}
			}

			velocity = cv;
			endVelocity = endClipVelocity;
			break;
		}
	}

	if (hasGravity)
		d->movementState.velocity = endVelocity;

	return (bumpcount != 0);
}

void GMBSPMove::clipVelocity(const GMVec3& in, const GMVec3& normal, GMVec3& out, GMfloat overbounce)
{
	GMfloat backoff;
	backoff = Dot(in, normal);

	if (backoff < 0) {
		backoff *= overbounce;
	}
	else {
		backoff /= overbounce;
	}

	out = in - normal * backoff;
}

void GMBSPMove::synchronizeMotionStates()
{
	D(d);
	GMMotionStates s = d->object->getMotionStates();
	s.transform = Translate(d->movementState.origin);
	s.linearVelocity = d->movementState.velocity;
	d->object->setMotionStates(s);
}
