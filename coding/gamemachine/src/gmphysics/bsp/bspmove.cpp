#include "stdafx.h"
#include "bspmove.h"
#include "gmengine/elements/bspgameworld.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmphysics/physicsstructs.h"
#include "gmengine/controllers/gamemachine.h"

enum
{
	GRAVITY_DIRECTION = 1
};

static const GMfloat OVERCLIP = 1.f;
static const GMfloat CLIP_IGNORE = .2f;

BSPMove::BSPMove(BSPPhysicsWorld* world, CollisionObject* obj)
{
	D(d);
	d->inited = false;
	d->world = world;
	d->object = obj;
	d->trace = &world->physicsData().trace;
	d->moveCommand.command = CMD_NONE;
}

void BSPMove::move()
{
	D(d);
	generateMovement();
	groundTrace();
	processCommand();
	if (d->movement.walking)
		walkMove();
	else
		airMove();
}

void BSPMove::processCommand()
{
	D(d);
	if (d->moveCommand.command & CMD_MOVE)
	{
		processMove();
		d->moveCommand.command &= ~CMD_MOVE;
	}
	else
	{
		//TODO 没有在move的时候，可以考虑摩擦使速度减小
		//这里我们先清空速度
		if (!d->movement.freefall)
			d->object->motions.velocity = linear_math::Vector3(0);
	}

	if (d->moveCommand.command & CMD_JUMP)
	{
		processJump();
		d->moveCommand.command &= ~CMD_JUMP;
	}
}

void BSPMove::processMove()
{
	D(d);
	// 空中不允许改变运动状态
	if (d->movement.freefall)
		return;

	//moveCommand: {pitch, yaw, USELESS}, {forward(bool), moveRate, USELESS}, {left(bool), moveRate(LR), USELESS}
	CommandVector3& arg0 = d->moveCommand.params[CMD_MOVE][0],
		&arg1 = d->moveCommand.params[CMD_MOVE][1],
		&arg2 = d->moveCommand.params[CMD_MOVE][2];
		
	GMfloat& pitch = arg0[0], &yaw = arg0[1];
	bool forward = arg1[0] == 1, left = arg2[0] == 1;
	GMfloat moveRate_fb = arg1[1], moveRate_lr = arg2[1];

	linear_math::Vector3 walkDirectionFB;
	{
		GMfloat distance = (forward ? 1 : -1) * d->object->motions.moveSpeed * moveRate_fb;
		GMfloat l = distance * cos(pitch);
		walkDirectionFB[0] = l * sin(yaw);
		walkDirectionFB[1] = distance * sin(pitch);
		walkDirectionFB[2] = -l * cos(yaw);
	}

	linear_math::Vector3 walkDirectionLR;
	{
		GMfloat distance = (left ? -1 : 1) * d->object->motions.moveSpeed * moveRate_lr;
		walkDirectionLR[0] = distance * cos(yaw);
		walkDirectionLR[1] = 0;
		walkDirectionLR[2] = distance * sin(yaw);
	}

	d->object->motions.velocity = linear_math::Vector3(
		walkDirectionFB[0] + walkDirectionLR[0],
		walkDirectionFB[1] + walkDirectionLR[1],
		walkDirectionFB[2] + walkDirectionLR[2]
	);

	composeVelocityWithGravity();
}

void BSPMove::processJump()
{
	D(d);
	if (!d->movement.freefall)
	{
		// 能够跳跃的场合
		d->movement.velocity[0] += d->object->motions.jumpSpeed[0];
		d->movement.velocity[1] = d->object->motions.jumpSpeed[1];
		d->movement.velocity[2] += d->object->motions.jumpSpeed[2];
	}
}

void BSPMove::sendCommand(Command cmd, const CommandParams& dataParam)
{
	D(d);
	d->moveCommand.command |= cmd;
	auto iter = dataParam.find(cmd);
	ASSERT(iter != dataParam.end());
	d->moveCommand.params[cmd] = (*iter).second;
}

GMfloat BSPMove::now()
{
	D(d);
	return d->world->physicsData().world->getElapsed();
}

void BSPMove::generateMovement()
{
	D(d);
	if (!d->inited)
	{
		memset(&d->movement, 0, sizeof(d->movement));
		d->movement.velocity = decomposeVelocity(d->object->motions.velocity);
		d->inited = true;
	}
	else
	{
		composeVelocityWithGravity();
	}

	d->movement.origin = d->object->motions.translation;
	d->movement.startTime = now();
}

void BSPMove::composeVelocityWithGravity()
{
	// 获取当前纵向速度，并叠加上加速度
	D(d);
	GMfloat accelerationVelocity = d->movement.velocity[GRAVITY_DIRECTION];
	d->movement.velocity = decomposeVelocity(d->object->motions.velocity);
	d->movement.velocity[GRAVITY_DIRECTION] = accelerationVelocity;
}

linear_math::Vector3 BSPMove::decomposeVelocity(const linear_math::Vector3& v)
{
	D(d);
	// 将速度分解成水平面平行的分量
	GMfloat len = linear_math::fast_length(v);
	linear_math::Vector3 planeDir = linear_math::Vector3(v[0], 0.f, v[2]);
	linear_math::Vector3 normal = linear_math::normalize(planeDir);
	return normal * len;
}

void BSPMove::groundTrace()
{
	D(d);
	linear_math::Vector3 p(d->movement.origin);
	p[1] -= .25f;

	d->trace->trace(d->movement.origin, p, linear_math::Vector3(0),
		d->object->shapeProps.bounding[0],
		d->object->shapeProps.bounding[1],
		d->movement.groundTrace
	);

	if (d->movement.groundTrace.fraction == 1.0)
	{
		// free fall
		d->movement.freefall = true;
		d->movement.walking = false;
		return;
	}

	d->movement.freefall = false;
	d->movement.walking = true;
}

void BSPMove::walkMove()
{
	D(d);
	if (linear_math::equals(d->movement.velocity, linear_math::Vector3(0)))
		return;

	stepSlideMove(false);
}

void BSPMove::airMove()
{
	stepSlideMove(true);
}

void BSPMove::stepSlideMove(bool hasGravity)
{
	D(d);
	linear_math::Vector3 startOrigin = d->movement.origin;
	linear_math::Vector3 startVelocity = d->movement.velocity;
	if (!slideMove(hasGravity))
	{
		synchronizePosition();
		return;
	}

	BSPTraceResult t;
	linear_math::Vector3 stepUp = startOrigin;
	stepUp[GRAVITY_DIRECTION] += d->object->shapeProps.stepHeight;
	d->trace->trace(d->movement.origin, stepUp, linear_math::Vector3(0), d->object->shapeProps.bounding[0], d->object->shapeProps.bounding[1], t);

	if (t.allsolid)
	{
		synchronizePosition();
		return;
	}

	// 从原位置stepUp
	d->movement.origin = t.endpos;
	d->movement.velocity = startVelocity;

	slideMove(hasGravity);

	// 走下来
	GMfloat stepSize = t.endpos[GRAVITY_DIRECTION] - startOrigin[GRAVITY_DIRECTION];
	linear_math::Vector3 stepDown = d->movement.origin;
	stepDown[GRAVITY_DIRECTION] -= stepSize;
	d->trace->trace(d->movement.origin, stepDown, linear_math::Vector3(0), d->object->shapeProps.bounding[0], d->object->shapeProps.bounding[1], t);
	if (!t.allsolid)
		d->movement.origin = t.endpos;
	if (t.fraction < 1.f)
	{
		clipVelocity(d->movement.velocity, t.plane.normal, d->movement.velocity, OVERCLIP);
		if (d->movement.velocity[GRAVITY_DIRECTION] < CLIP_IGNORE)
			d->movement.velocity[GRAVITY_DIRECTION] = 0;
	}
	synchronizePosition();
}

bool BSPMove::slideMove(bool hasGravity)
{
	D(d);
	BSPPhysicsWorld::Data& wd = d->world->physicsData();
	GMfloat dt = wd.world->getGameMachine()->evaluateDeltaTime();
	linear_math::Vector3 velocity = d->movement.velocity;

	GMint numbumps = 4, bumpcount;
	linear_math::Vector3 endVelocity, endClipVelocity;
	if (hasGravity)
	{
		endVelocity = velocity;
		endVelocity[GRAVITY_DIRECTION] += wd.gravity * dt;
		velocity[GRAVITY_DIRECTION] = (velocity[GRAVITY_DIRECTION] + endVelocity[GRAVITY_DIRECTION]) * .5f;
	}

	velocity *= dt;

	AlignedVector<linear_math::Vector3> planes;
	if (!d->movement.freefall)
		planes.push_back(d->movement.groundTrace.plane.normal);

	planes.push_back(linear_math::normalize(velocity));

	GMfloat t = 1.0f;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		BSPTraceResult moveTrace;
		d->trace->trace(d->movement.origin,
			d->movement.origin + velocity * t,
			linear_math::Vector3(0, 0, 0),
			d->object->shapeProps.bounding[0],
			d->object->shapeProps.bounding[1],
			moveTrace
		);
		
		if (moveTrace.allsolid)
		{
			// entity is completely trapped in another solid
			d->movement.velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
			return true;
		}
		if (moveTrace.fraction > 0)
			d->movement.origin = moveTrace.endpos;
		if (moveTrace.fraction == 1.0f)
			break;

		t -= t * moveTrace.fraction;

		GMuint i;
		for (i = 0; i < planes.size(); i++)
		{
			if (linear_math::dot(moveTrace.plane.normal, planes[i]) > 0.99)
				velocity += moveTrace.plane.normal;
		}
		if (i < planes.size())
			continue;

		planes.push_back(moveTrace.plane.normal);

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		linear_math::Vector3 cv; //clipVelocity
		for (i = 0; i < planes.size(); i++)
		{
			if (linear_math::dot(velocity, planes[i]) >= 0.1)
				continue; // 朝着平面前方移动，不会有交汇
			clipVelocity(velocity, planes[i], cv, OVERCLIP);
			clipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

			for (GMuint j = 0; j < planes.size(); j++)
			{
				if (i == j)
					continue;
				if (linear_math::dot(cv, planes[j]) >= 0.1)
					continue;

				// try clipping the move to the plane
				clipVelocity(cv, planes[j], cv, OVERCLIP);
				clipVelocity(endClipVelocity, planes[i], endClipVelocity, OVERCLIP);

				// see if it goes back into the first clip plane
				if (linear_math::dot(cv, planes[i]) >= 0)
					continue;

				// slide the original velocity along the crease
				{
					linear_math::Vector3 dir = linear_math::cross(planes[i], planes[j]);
					dir = linear_math::normalize(dir);
					GMfloat s = linear_math::dot(dir, velocity);
					cv = dir * s;
				}

				{
					linear_math::Vector3 dir = linear_math::cross(planes[i], planes[j]);
					dir = linear_math::normalize(dir);
					GMfloat s = linear_math::dot(dir, endVelocity);
					endClipVelocity = dir * s;
				}

				for (GMuint k = 0; k < planes.size(); k++)
				{
					if (k == i || k == j)
						continue;

					if (linear_math::dot(cv, planes[k]) >= 0.1)
						continue;

					velocity = linear_math::Vector3(0);
					return true;
				}
			}

			velocity = cv;
			endVelocity = endClipVelocity;
			break;
		}
	}

	if (hasGravity)
		d->movement.velocity = endVelocity;

	return (bumpcount != 0);
}

void BSPMove::clipVelocity(const linear_math::Vector3& in, const linear_math::Vector3& normal, linear_math::Vector3& out, GMfloat overbounce)
{
	GMfloat backoff;
	GMfloat change;
	GMint i;

	backoff = linear_math::dot(in, normal);

	if (backoff < 0) {
		backoff *= overbounce;
	}
	else {
		backoff /= overbounce;
	}

	for (i = 0; i < 3; i++) {
		change = normal[i] * backoff;
		out[i] = in[i] - change;
	}
}

void BSPMove::synchronizePosition()
{
	D(d);
	d->object->motions.translation = d->movement.origin;
}
