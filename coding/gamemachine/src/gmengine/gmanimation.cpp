#include "stdafx.h"
#include "gmengine/gameobjects/gmgameobject.h"
#include "gmanimation.h"
#include "foundation/gamemachine.h"
#include "gameobjects/gmcontrolgameobject.h"

GMAnimation::GMAnimation(GMGameObject* object)
{
	D(d);
	d->object = object;
}

void GMAnimation::start()
{
	D(d);
	if (d->canStart)
	{
		GM_FOREACH_ENUM(type, GMAnimationTypes::BeginType, GMAnimationTypes::EndType)
		{
			GMAnimationState& state = d->animationStates[type];
			if (state.set)
				startAnimation(type);
		}
	}
}

void GMAnimation::reverse()
{
	D(d);
	if (d->canReverse)
	{
		GM_FOREACH_ENUM(type, GMAnimationTypes::BeginType, GMAnimationTypes::EndType)
		{
			GMAnimationState& state = d->animationStates[type];
				state.direction = -1;
				d->canResume = true;
		}
	}
}

void GMAnimation::resume()
{
	D(d);
	if (d->canResume)
	{
		GM_FOREACH_ENUM(type, GMAnimationTypes::BeginType, GMAnimationTypes::EndType)
		{
			GMAnimationState& state = d->animationStates[type];
			startAnimation(type);
		}
	}
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
			state.p += state.direction * (now - state.tick) / d->duration;
			if (state.p >= 1.f)
			{
				state.p = 1.f;
				d->canReverse = true;
				d->canResume = d->canStart = false;
			}
			else if (state.p < 0)
			{
				state.p = 0;
				d->canResume = d->canReverse = false;
				d->canStart = true;
				state.playingState = GMAnimationPlayingState::Deactivated;
			}

			if (type != GMAnimationTypes::Rotation)
			{
				linear_math::Vector3 p = state.interpolation(state.start, state.end, state.p);
				linear_math::Matrix4x4 s = transformFunc[type](p);
				transformFunSetList[type](d->object, s);
			}
			else
			{
				linear_math::Quaternion p = state.interpolation_q(state.start_q, state.end_q, state.p);
				gm_info("%f, %f, %f, %f", p[0], p[1], p[2], p[3]);
				d->object->setRotation(p);
			}
		}
		state.tick = now;
	}
}

void GMAnimation::setDuration(GMfloat duration)
{
	D(d);
	d->duration = duration;
}

void GMAnimation::setScaling(const linear_math::Vector3& scaling, GMInterpolation interpolation)
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

void GMAnimation::setTranslation(const linear_math::Vector3& translation, GMInterpolation interpolation)
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Translation];
	state.interpolation = interpolation;
	auto& translationMatrix = d->object->getTranslation();
	GMfloat s[4];
	linear_math::getTranslationFromMatrix(translationMatrix, s);
	state.start[0] = s[0];
	state.start[1] = s[1];
	state.start[2] = s[2];
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

void GMAnimation::setRotation(const linear_math::Quaternion& rotation, GMQuaternionInterpolation interpolation)
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Rotation];
	state.interpolation_q = interpolation;
	state.start_q = d->object->getRotation();
	state.p = 0;
	state.end_q = rotation;
	state.direction = 1;
	state.set = true;
}

void GMAnimation::disableRotation()
{
	D(d);
	GMAnimationState& state = d->animationStates[GMAnimationTypes::Rotation];
	state.set = false;
}

void GMAnimation::startAnimation(GMAnimationTypes::Types type)
{
	D(d);
	GMAnimationState& state = d->animationStates[type];
	state.tick = GM.getGameTimeSeconds();
	state.direction = 1;

	d->canReverse = d->canResume = true;
	d->canStart = false;
	state.playingState = GMAnimationPlayingState::Activated;
}

//////////////////////////////////////////////////////////////////////////
GMControlGameObjectAnimation::GMControlGameObjectAnimation(GMControlGameObject* object)
	: GMAnimation(object)
{
	D(d);
	d->object = object;
}

void GMControlGameObjectAnimation::setTranslation(GMint x, GMint y, GMInterpolation interpolation)
{
	D(d);
	D_BASE(db, GMAnimation);
	// translation是控件空间，左上角(0,0)，变换到绘制空间
	const GMRect& geometry = d->object->getGeometry();
	GMRect viewportCoord = { x + geometry.width / 2, y + geometry.height / 2 };
	GMRectF coord = GMControlGameObject::toViewportCoord(viewportCoord);
	linear_math::Vector3 trans(coord.x, coord.y, 0);

	GMAnimationState& state = db->animationStates[GMAnimationTypes::Translation];
	state.interpolation = interpolation;
	auto& translationMatrix = d->object->getTranslation();
	GMfloat s[3];
	linear_math::getTranslationFromMatrix(translationMatrix, s);
	state.start[0] = s[0];
	state.start[1] = s[1];
	state.start[2] = s[2];
	state.p = 0;
	state.end = trans;
	state.direction = 1;
	state.set = true;
}