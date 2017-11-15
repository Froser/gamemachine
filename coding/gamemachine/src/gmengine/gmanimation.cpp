#include "stdafx.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmanimation.h"
#include "foundation/gamemachine.h"

GMAnimation::GMAnimation(GMGameObject* object)
{
	D(d);
	d->object = object;
}

bool GMAnimation::canStart()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	return state.canStart;
}

void GMAnimation::start()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	if (state.canStart)
		startAnimation();
}

bool GMAnimation::canReverse()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	return state.canReverse;
}

void GMAnimation::reverse()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	if (state.canReverse)
	{
		state.direction = -1;
		state.canResume = true;
	}
}

void GMAnimation::resume()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	if (state.canResume)
		startAnimation();
}

bool GMAnimation::canResume()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	return state.canResume;
}

void GMAnimation::update()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	GMfloat now = GM.getGameTimeSeconds();
	if (state.playingState == GMAnimationPlayingState::Activated)
	{
		state.p += state.direction * (now - state.tick) / state.duration;
		if (state.p >= 1.f)
		{
			state.p = 1.f;
			state.canReverse = true;
			state.canResume = state.canStart = false;
		}
		else if (state.p < 0)
		{
			state.p = 0;
			state.canResume = state.canReverse = false;
			state.canStart = true;
			state.playingState = GMAnimationPlayingState::Deactivated;
		}

		auto scaling = state.interpolation(state.start, state.end, state.p);
		linear_math::Matrix4x4 s = linear_math::scale(scaling);
		d->object->setScaling(s);
	}
	state.tick = now;
}

void GMAnimation::setScaling(const linear_math::Vector3& endScaling, GMfloat duration, GMInterpolation interpolation)
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	state.interpolation = interpolation;
	auto& scalingMatrix = d->object->getScaling();
	state.start[0] = scalingMatrix[0][0];
	state.start[1] = scalingMatrix[1][1];
	state.start[2] = scalingMatrix[2][2];
	state.duration = duration;
	state.p = 0;
	state.end = endScaling;
	state.direction = 1;
}

void GMAnimation::startAnimation()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	state.tick = GM.getGameTimeSeconds();
	state.direction = 1;

	state.canReverse = state.canResume = true;
	state.canStart = false;
	state.playingState = GMAnimationPlayingState::Activated;
}