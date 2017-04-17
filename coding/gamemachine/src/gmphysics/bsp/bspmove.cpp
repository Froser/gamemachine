#include "stdafx.h"
#include "bspmove.h"
#include "gmengine/controllers/gameloop.h"
#include "gmengine/elements/bspgameworld.h"
#include "gmengine/controllers/graphic_engine.h"
#include "gmphysics/physicsstructs.h"

enum
{
	GRAVITY_DIRECTION = 1
};

static const GMfloat OVERCLIP = 1.01f;

BSPMove::BSPMove(BSPPhysicsWorld* world, CollisionObject* obj)
{
	D(d);
	d.inited = false;
	d.world = world;
	d.object = obj;
	d.trace = &world->physicsData().trace;
}

void BSPMove::move()
{
	D(d);
	generateMovement();
	groundTrace();
	processCommand();
	if (d.movement.walking)
		walkMove();
	else
		airMove();
}

void BSPMove::processCommand()
{
	D(d);
	if (d.moveCommand.command & CMD_JUMP)
	{
		processJump();
		d.moveCommand.command &= ~CMD_JUMP;
	}
}

void BSPMove::processJump()
{
	D(d);
	if (!d.movement.freefall)
	{
		// 能够跳跃的场合
		d.movement.velocity = *(vmath::vec3*)d.moveCommand.data;
	}
}

void BSPMove::sendCommand(Command cmd, void* dataParam)
{
	D(d);
	d.moveCommand.command |= cmd;
	d.moveCommand.data = dataParam;
}

GMfloat BSPMove::now()
{
	D(d);
	return d.world->physicsData().world->getElapsed();
}

void BSPMove::generateMovement()
{
	D(d);
	if (!d.inited)
	{
		memset(&d.movement, 0, sizeof(d.movement));
		d.movement.velocity = decomposeVelocity(d.object->motions.velocity);
		d.inited = true;
		d.moveCommand.command = CMD_NONE;
	}
	else
	{
		composeVelocityWithGravity();
	}

	memset(&d.movement.groundTrace, 0, sizeof(d.movement.groundTrace));
	d.movement.origin = d.object->motions.translation;
	d.movement.startTime = now();
}

void BSPMove::composeVelocityWithGravity()
{
	// 获取当前纵向速度，并叠加上加速度
	D(d);
	GMfloat gravityVelocity = d.movement.velocity[GRAVITY_DIRECTION];
	d.movement.velocity = decomposeVelocity(d.object->motions.velocity);
	d.movement.velocity[GRAVITY_DIRECTION] = gravityVelocity;
}

vmath::vec3 BSPMove::decomposeVelocity(const vmath::vec3& v)
{
	D(d);
	// 将速度分解成水平面平行的分量
	GMfloat len = vmath::fast_length(v);
	vmath::vec3 planeDir = vmath::vec3(v[0], 0.f, v[2]);
	vmath::vec3 normal = vmath::normalize(planeDir);
	return normal * len;
}

void BSPMove::groundTrace()
{
	D(d);
	vmath::vec3 p(d.movement.origin);
	p[1] -= .25f;

	d.trace->trace(d.movement.origin, p, vmath::vec3(0),
		d.object->shapeProps.bounding[0],
		d.object->shapeProps.bounding[1],
		d.movement.groundTrace
	);

	if (d.movement.groundTrace.fraction == 1.0)
	{
		// free fall
		d.movement.freefall = true;
		d.movement.walking = false;
		return;
	}

	d.movement.freefall = false;
	d.movement.walking = true;
}

void BSPMove::walkMove()
{
	D(d);
	if (vmath::equals(d.movement.velocity, vmath::vec3(0)))
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
	vmath::vec3 startOrigin = d.movement.origin;
	vmath::vec3 startVelocity = d.movement.velocity;
	if (!slideMove(hasGravity))
	{
		synchronizePosition();
		return;
	}

	BSPTraceResult t;

	// 看看是否拥有向上速度，如果有，则不stepUp
	vmath::vec3 stepDown = d.movement.origin;
	stepDown[GRAVITY_DIRECTION] -= d.object->shapeProps.stepHeight;
	d.trace->trace(startOrigin, stepDown, vmath::vec3(0), d.object->shapeProps.bounding[0], d.object->shapeProps.bounding[1], t);
	if (d.movement.velocity[GRAVITY_DIRECTION] > 0 && (t.fraction == 1.f || vmath::dot(t.plane.normal, vmath::vec3(0, 1, 0)) < .7f) )
	{
		synchronizePosition();
		return;
	}

	vmath::vec3 stepUp = startOrigin;
	stepUp[GRAVITY_DIRECTION] += d.object->shapeProps.stepHeight;
	d.trace->trace(d.movement.origin, stepUp, vmath::vec3(0), d.object->shapeProps.bounding[0], d.object->shapeProps.bounding[1], t);

	if (t.allsolid)
	{
		synchronizePosition();
		return;
	}

	// 从原位置stepUp
	d.movement.origin = t.endpos;
	d.movement.velocity = startVelocity;
	composeVelocityWithGravity();

	slideMove(hasGravity);

	// 走下来
	GMfloat stepSize = t.endpos[GRAVITY_DIRECTION] - startOrigin[GRAVITY_DIRECTION];
	stepDown = d.movement.origin;
	stepDown[GRAVITY_DIRECTION] -= stepSize;
	d.trace->trace(d.movement.origin, stepDown, vmath::vec3(0), d.object->shapeProps.bounding[0], d.object->shapeProps.bounding[1], t);
	if (!t.allsolid)
		d.movement.origin = t.endpos;
	if (t.fraction < 1.f)
		clipVelocity(d.movement.velocity, t.plane.normal, d.movement.velocity, OVERCLIP);
	synchronizePosition();
}

bool BSPMove::slideMove(bool hasGravity)
{
	D(d);
	BSPPhysicsWorldData& wd = d.world->physicsData();
	GMfloat elapsed = GameLoop::getInstance()->getElapsedAfterLastFrame();
	vmath::vec3 velocity = d.movement.velocity;

	GMint numbumps = 4, bumpcount;
	vmath::vec3 endVelocity, endClipVelocity;
	if (hasGravity)
	{
		endVelocity = velocity;
		endVelocity[GRAVITY_DIRECTION] += wd.gravity * elapsed;
		velocity[GRAVITY_DIRECTION] = (velocity[GRAVITY_DIRECTION] + endVelocity[GRAVITY_DIRECTION]) * .5f;
	}

	velocity *= elapsed;

	std::vector<vmath::vec3> planes;
	if (!d.movement.freefall)
		planes.push_back(d.movement.groundTrace.plane.normal);

	planes.push_back(vmath::normalize(velocity));

	GMfloat t = 1.0f;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		BSPTraceResult moveTrace;
		d.trace->trace(d.movement.origin,
			d.movement.origin + velocity * t,
			vmath::vec3(0, 0, 0),
			d.object->shapeProps.bounding[0],
			d.object->shapeProps.bounding[1],
			moveTrace
			);
		
		if (moveTrace.allsolid)
		{
			// entity is completely trapped in another solid
			d.movement.velocity[2] = 0;	// don't build up falling damage, but allow sideways acceleration
			return true;
		}
		if (moveTrace.fraction > 0)
			d.movement.origin = moveTrace.endpos;
		if (moveTrace.fraction == 1.0f)
			break;

		t -= t * moveTrace.fraction;

		GMuint i;
		for (i = 0; i < planes.size(); i++)
		{
			if (vmath::dot(moveTrace.plane.normal, planes[i]) > 0.99)
				velocity += moveTrace.plane.normal;
		}
		if (i < planes.size())
			continue;

		planes.push_back(moveTrace.plane.normal);

		//
		// modify velocity so it parallels all of the clip planes
		//

		// find a plane that it enters
		vmath::vec3 cv; //clipVelocity
		for (i = 0; i < planes.size(); i++)
		{
			if (vmath::dot(velocity, planes[i]) >= 0.1)
				continue; // 朝着平面前方移动，不会有交汇
			clipVelocity(velocity, planes[i], cv, OVERCLIP);
			clipVelocity(endVelocity, planes[i], endClipVelocity, OVERCLIP);

			for (GMuint j = 0; j < planes.size(); j++)
			{
				if (i == j)
					continue;
				if (vmath::dot(cv, planes[j]) >= 0.1)
					continue;

				// try clipping the move to the plane
				clipVelocity(cv, planes[j], cv, OVERCLIP);
				clipVelocity(endClipVelocity, planes[i], endClipVelocity, OVERCLIP);

				// see if it goes back into the first clip plane
				if (vmath::dot(cv, planes[i]) >= 0)
					continue;

				// slide the original velocity along the crease
				{
					vmath::vec3 dir = vmath::cross(planes[i], planes[j]);
					dir = vmath::normalize(dir);
					GMfloat s = vmath::dot(dir, velocity);
					cv = dir * s;
				}

				{
					vmath::vec3 dir = vmath::cross(planes[i], planes[j]);
					dir = vmath::normalize(dir);
					GMfloat s = vmath::dot(dir, endVelocity);
					endClipVelocity = dir * s;
				}

				for (GMuint k = 0; k < planes.size(); k++)
				{
					if (k == i || k == j)
						continue;

					if (vmath::dot(cv, planes[k]) >= 0.1)
						continue;

					velocity = vmath::vec3(0);
					return true;
				}
			}

			velocity = cv;
			endVelocity = endClipVelocity;
			d.movement.velocity = velocity;
			break;
		}
	}

	if (hasGravity)
		d.movement.velocity = endVelocity;

	return (bumpcount != 0);
}

void BSPMove::clipVelocity(const vmath::vec3& in, const vmath::vec3& normal, vmath::vec3& out, GMfloat overbounce)
{
	GMfloat backoff;
	GMfloat change;
	GMint i;

	backoff = vmath::dot(in, normal);

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
	d.object->motions.translation = d.movement.origin;
}