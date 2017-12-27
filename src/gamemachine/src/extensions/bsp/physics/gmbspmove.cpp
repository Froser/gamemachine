#include "stdafx.h"
#include "gmbspmove.h"
#include "extensions/bsp/render/gmbspgameworld.h"
#include <gmbspphysicsworld.h>
#include "gmphysics/gmphysicsstructs.h"
#include "foundation/gamemachine.h"
#include "foundation/gmprofile.h"

enum
{
	GRAVITY_DIRECTION = 1
};

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

void GMBSPMove::move()
{
	D(d);
	generateMovement();
	groundTrace();
	processCommand();
	if (d->movementState.walking)
		walkMove();
	else
		airMove();
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
			d->object->motions.velocity = glm::vec3(0);
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

	const GMPhysicsMoveArgs& moveArgs = d->action.move.args;
	//moveArgs.direction
	
	/*
	//moveCommand: {pitch, yaw, USELESS}, {forward(bool), moveRate, USELESS}, {left(bool), moveRate(LR), USELESS}
	GMCommandVector3& arg0 = d->moveCommand.params[CMD_MOVE][0],
		&arg1 = d->moveCommand.params[CMD_MOVE][1],
		&arg2 = d->moveCommand.params[CMD_MOVE][2];
		
	GMfloat& pitch = arg0[0], &yaw = arg0[1];
	bool forward = arg1[0] == 1, left = arg2[0] == 1;
	GMfloat moveRate_fb = arg1[1], moveRate_lr = arg2[1];

	glm::vec3 walkDirectionFB;
	{
		GMfloat distance = (forward ? 1 : -1) * d->object->motions.moveSpeed * moveRate_fb;
		GMfloat l = distance * cos(pitch);
		walkDirectionFB[0] = l * sin(yaw);
		walkDirectionFB[1] = distance * sin(pitch);
		walkDirectionFB[2] = -l * cos(yaw);
	}

	glm::vec3 walkDirectionLR;
	{
		GMfloat distance = (left ? -1 : 1) * d->object->motions.moveSpeed * moveRate_lr;
		walkDirectionLR[0] = distance * cos(yaw);
		walkDirectionLR[1] = 0;
		walkDirectionLR[2] = distance * sin(yaw);
	}

	d->object->motions.velocity = glm::vec3(
		walkDirectionFB[0] + walkDirectionLR[0],
		walkDirectionFB[1] + walkDirectionLR[1],
		walkDirectionFB[2] + walkDirectionLR[2]
	);
	*/

	composeVelocityWithGravity();
}

void GMBSPMove::processJump()
{
	D(d);
	if (!d->movementState.freefall)
	{
		// 能够跳跃的场合
		d->movementState.velocity[0] += d->object->motions.jumpSpeed[0];
		d->movementState.velocity[1] = d->object->motions.jumpSpeed[1];
		d->movementState.velocity[2] += d->object->motions.jumpSpeed[2];
	}
}

void GMBSPMove::applyMove(const GMPhysicsMoveArgs& args)
{
	D(d);
	d->action.move.moved = true;
	d->action.move.args = args;
}

void GMBSPMove::applyJump()
{
	D(d);
	d->action.jump.jumped = true;
}

GMfloat GMBSPMove::now()
{
	D(d);
	return GameMachine::instance().getGameTimeSeconds();
}

void GMBSPMove::generateMovement()
{
	D(d);
	if (!d->inited)
	{
		d->movementState.velocity = decomposeVelocity(d->object->motions.velocity);
		d->inited = true;
	}
	else
	{
		composeVelocityWithGravity();
	}

	d->movementState.origin = d->object->motions.translation;
	d->movementState.startTime = now();
}

void GMBSPMove::composeVelocityWithGravity()
{
	// 获取当前纵向速度，并叠加上加速度
	D(d);
	GMfloat accelerationVelocity = d->movementState.velocity[GRAVITY_DIRECTION];
	d->movementState.velocity = decomposeVelocity(d->object->motions.velocity);
	d->movementState.velocity[GRAVITY_DIRECTION] = accelerationVelocity;
}

glm::vec3 GMBSPMove::decomposeVelocity(const glm::vec3& v)
{
	D(d);
	// 将速度分解成水平面平行的分量
	GMfloat len = glm::fastLength(v);
	glm::vec3 planeDir = glm::vec3(v[0], 0.f, v[2]);
	glm::vec3 normal = glm::fastNormalize(planeDir);
	return normal * len;
}

void GMBSPMove::groundTrace()
{
	D(d);
	glm::vec3 p(d->movementState.origin);
	p[1] -= .25f;

	d->trace->trace(d->movementState.origin, p, glm::vec3(0),
		d->object->shapeProps.bounding[0],
		d->object->shapeProps.bounding[1],
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

void GMBSPMove::walkMove()
{
	D(d);
	if (d->movementState.velocity == glm::vec3(0))
		return;

	stepSlideMove(false);
}

void GMBSPMove::airMove()
{
	stepSlideMove(true);
}

void GMBSPMove::stepSlideMove(bool hasGravity)
{
	D(d);
	glm::vec3 startOrigin = d->movementState.origin;
	glm::vec3 startVelocity = d->movementState.velocity;
	if (!slideMove(hasGravity))
	{
		synchronizePosition();
		return;
	}

	BSPTraceResult t;
	glm::vec3 stepUp = startOrigin;
	stepUp[GRAVITY_DIRECTION] += d->object->shapeProps.stepHeight;
	d->trace->trace(d->movementState.origin, stepUp, glm::vec3(0), d->object->shapeProps.bounding[0], d->object->shapeProps.bounding[1], t);

	if (t.allsolid)
	{
		synchronizePosition();
		return;
	}

	// 从原位置stepUp
	d->movementState.origin = t.endpos;
	d->movementState.velocity = startVelocity;

	slideMove(hasGravity);

	// 走下来
	GMfloat stepSize = t.endpos[GRAVITY_DIRECTION] - startOrigin[GRAVITY_DIRECTION];
	glm::vec3 stepDown = d->movementState.origin;
	stepDown[GRAVITY_DIRECTION] -= stepSize;
	d->trace->trace(d->movementState.origin, stepDown, glm::vec3(0), d->object->shapeProps.bounding[0], d->object->shapeProps.bounding[1], t);
	if (!t.allsolid)
		d->movementState.origin = t.endpos;
	if (t.fraction < 1.f)
	{
		clipVelocity(d->movementState.velocity, t.plane.normal, d->movementState.velocity, OVERCLIP);
		if (d->movementState.velocity[GRAVITY_DIRECTION] < CLIP_IGNORE)
			d->movementState.velocity[GRAVITY_DIRECTION] = 0;
	}
	synchronizePosition();
}

bool GMBSPMove::slideMove(bool hasGravity)
{
	D(d);
	GMBSPPhysicsWorld::Data& wd = d->world->physicsData();
	GMfloat dt = GM.getGameMachineRunningStates().lastFrameElpased;
	glm::vec3 velocity = d->movementState.velocity;

	GMint numbumps = 4, bumpcount;
	glm::vec3 endVelocity, endClipVelocity;
	if (hasGravity)
	{
		endVelocity = velocity;
		endVelocity[GRAVITY_DIRECTION] += wd.gravity * dt;
		velocity[GRAVITY_DIRECTION] = (velocity[GRAVITY_DIRECTION] + endVelocity[GRAVITY_DIRECTION]) * .5f;
	}

	velocity *= dt;

	AlignedVector<glm::vec3> planes;
	if (!d->movementState.freefall)
		planes.push_back(d->movementState.groundTrace.plane.normal);

	planes.push_back(glm::fastNormalize(velocity));

	GMfloat t = 1.0f;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		BSPTraceResult moveTrace;
		d->trace->trace(d->movementState.origin,
			d->movementState.origin + velocity * t,
			glm::vec3(0, 0, 0),
			d->object->shapeProps.bounding[0],
			d->object->shapeProps.bounding[1],
			moveTrace
		);
		
		if (moveTrace.allsolid)
		{
			// entity is completely trapped in another solid
			d->movementState.velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
			return true;
		}
		if (moveTrace.fraction > 0)
			d->movementState.origin = moveTrace.endpos;
		if (moveTrace.fraction == 1.0f)
			break;

		t -= t * moveTrace.fraction;

		GMuint i;
		for (i = 0; i < planes.size(); i++)
		{
			if (glm::dot(moveTrace.plane.normal, planes[i]) > 0.99)
				velocity += moveTrace.plane.normal;
		}
		if (i < planes.size())
			continue;

		planes.push_back(moveTrace.plane.normal);

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		glm::vec3 cv; //clipVelocity
		for (i = 0; i < planes.size(); i++)
		{
			if (glm::dot(velocity, planes[i]) >= 0.1)
				continue; // 朝着平面前方移动，不会有交汇
			clipVelocity(velocity, planes[i], cv, OVERCLIP);
			clipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

			for (GMuint j = 0; j < planes.size(); j++)
			{
				if (i == j)
					continue;
				if (glm::dot(cv, planes[j]) >= 0.1)
					continue;

				// try clipping the move to the plane
				clipVelocity(cv, planes[j], cv, OVERCLIP);
				clipVelocity(endClipVelocity, planes[i], endClipVelocity, OVERCLIP);

				// see if it goes back into the first clip plane
				if (glm::dot(cv, planes[i]) >= 0)
					continue;

				// slide the original velocity along the crease
				{
					glm::vec3 dir = glm::cross(planes[i], planes[j]);
					dir = glm::fastNormalize(dir);
					GMfloat s = glm::dot(dir, velocity);
					cv = dir * s;
				}

				{
					glm::vec3 dir = glm::cross(planes[i], planes[j]);
					dir = glm::fastNormalize(dir);
					GMfloat s = glm::dot(dir, endVelocity);
					endClipVelocity = dir * s;
				}

				for (GMuint k = 0; k < planes.size(); k++)
				{
					if (k == i || k == j)
						continue;

					if (glm::dot(cv, planes[k]) >= 0.1)
						continue;

					velocity = glm::vec3(0);
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

void GMBSPMove::clipVelocity(const glm::vec3& in, const glm::vec3& normal, glm::vec3& out, GMfloat overbounce)
{
	GMfloat backoff;
	GMfloat change;
	GMint i;

	backoff = glm::dot(in, normal);

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

void GMBSPMove::synchronizePosition()
{
	D(d);
	d->object->motions.translation = d->movementState.origin;
}
