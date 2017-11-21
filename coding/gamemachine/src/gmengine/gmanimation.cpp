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
	GM_FOREACH_ENUM(type, GMAnimationTypes::BeginType, GMAnimationTypes::EndType)
	{
		GMAnimationState& state = d->animationStates[type];
		if (state.canReverse)
		{
			state.direction = -1;
			state.canResume = true;
		}
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
	GMfloat now = GM.getGameTimeSeconds();

	decltype(std::mem_fn(&GMGameObject::setScaling)) transformFunSetList[] = {
		std::mem_fn(&GMGameObject::setScaling),
		std::mem_fn(&GMGameObject::setTranslation),
	};

	typedef linear_math::Matrix4x4(*TransformFunc)(const linear_math::Vector3&);
	TransformFunc transformFunc[] {
		linear_math::scale,
		linear_math::translate,
	};

	GM_FOREACH_ENUM(type, GMAnimationTypes::BeginType, GMAnimationTypes::EndType)
	{
		GMAnimationState& state = d->animationStates[type];
		if (state.set && state.playingState == GMAnimationPlayingState::Activated)
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

			auto p = state.interpolation(state.start, state.end, state.p);
			linear_math::Matrix4x4 s = transformFunc[type](p);
			transformFunSetList[type](d->object, s);
		}
		state.tick = now;
	}
}

void GMAnimation::setScaling(const linear_math::Vector3& scaling, GMfloat duration, GMInterpolation interpolation)
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	state.interpolation = interpolation;
	auto& scalingMatrix = d->object->getScaling();
	GMfloat s[3];
	linear_math::getScalingFromMatrix(scalingMatrix, s);
	state.start[0] = s[0];
	state.start[1] = s[1];
	state.start[2] = s[2];
	state.duration = duration;
	state.p = 0;
	state.end = scaling;
	state.direction = 1;
	state.set = true;
}

void GMAnimation::disableScaling()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Scaling];
	state.set = false;
}

void GMAnimation::setTranslation(const linear_math::Vector3& translation, GMfloat duration, GMInterpolation interpolation)
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Translation];
	state.interpolation = interpolation;
	auto& translationMatrix = d->object->getTranslation();
	GMfloat s[3];
	linear_math::getTranslationFromMatrix(translationMatrix, s);
	state.start[0] = s[0];
	state.start[1] = s[1];
	state.start[2] = s[2];
	state.duration = duration;
	state.p = 0;
	state.end = translation;
	state.direction = 1;
	state.set = true;
}

void GMAnimation::disableTranslation()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Translation];
	state.set = false;
}

void GMAnimation::startAnimation()
{
	D(d);
	GM_FOREACH_ENUM(type, GMAnimationTypes::BeginType, GMAnimationTypes::EndType)
	{
		GMAnimationState& state = d->animationStates[type];
		state.tick = GM.getGameTimeSeconds();
		state.direction = 1;

		state.canReverse = state.canResume = true;
		state.canStart = false;
		state.playingState = GMAnimationPlayingState::Activated;
	}
}
