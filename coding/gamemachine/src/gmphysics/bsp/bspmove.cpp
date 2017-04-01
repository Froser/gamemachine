#include "stdafx.h"
#include "bspmove.h"
#include "gmengine/controllers/gameloop.h"
#include "gmengine/elements/bspgameworld.h"
#include "gmengine/controllers/graphic_engine.h"

BSPMove::BSPMove(BSPPhysicsWorld* world, CollisionObject* obj)
{
	D(d);
	d.world = world;
	d.object = obj;
}

void BSPMove::slideMove(bool hasGravity)
{
	D(d);
	BSPPhysicsWorldData& wd = d.world->physicsData();
	GMfloat elapsed = GameLoop::getInstance()->getElapsedAfterLastFrame();
	GMfloat fps = wd.world->getGraphicEngine()->getGraphicSettings()->fps;
	GMfloat skipFrame = elapsed / (1.0f / fps);
	if (skipFrame < 1)
		skipFrame = 1;

	vmath::vec3 velocity = d.object->motions.velocity * skipFrame / fps;
	GMint numbumps = 4, bumpcount;

	vmath::vec3 endVelocity, endClipVelocity;
	if (hasGravity)
	{
		endVelocity = wd.gravity;//TODO *skipFrame / fps;
		velocity += endVelocity;
	}

	std::vector<vmath::vec3> planes;
	//planes.push_back(groundTrace.plane.normal); //TODO
	planes.push_back(vmath::normalize(velocity));

	GMfloat t = 1.0f;

	for (bumpcount = 0; bumpcount < numbumps; bumpcount++)
	{
		BSPTraceResult moveTrace;
		wd.trace.trace(d.object->motions.translation,
			d.object->motions.translation + velocity * t,
			vmath::vec3(0, 0, 0),
			vmath::vec3(-15),
			vmath::vec3(15),
			moveTrace
			);
		//TODO -15和15应该取自CollisionObject的形状参数

		if (moveTrace.fraction > 0)
			d.object->motions.translation = moveTrace.endpos;
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
		const GMfloat OVERCLIP = 1.01f;
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
					return;
				}
			}

			velocity = cv;
			endVelocity = endClipVelocity;
			break;
		}
	}

	if (hasGravity)
		velocity = endVelocity;
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
