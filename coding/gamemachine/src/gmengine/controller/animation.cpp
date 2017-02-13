#include "stdafx.h"
#include "animation.h"
#include "utilities\assert.h"
#include "utilities\algorithm.h"

static vmath::quaternion interpolation(GMfloat v,
	const Keyframe* lower, 
	const KeyframeMovement& lowerMovement, 
	const Keyframe* upper, 
	const KeyframeMovement& upperMovement,
	InterpolationFunctor functor)
{
	GMfloat l = lower->percentage, u = upper->percentage;

	GMfloat x = functor(InterpolationVector2(l, lowerMovement.direction[0]), InterpolationVector2(u, upperMovement.direction[0]), v);
	GMfloat y = functor(InterpolationVector2(l, lowerMovement.direction[1]), InterpolationVector2(u, upperMovement.direction[1]), v);
	GMfloat z = functor(InterpolationVector2(l, lowerMovement.direction[2]), InterpolationVector2(u, upperMovement.direction[2]), v);
	GMfloat a = functor(InterpolationVector2(l, lowerMovement.value), InterpolationVector2(u, upperMovement.value), v);

	return vmath::quaternion(x, y, z, a);
}

Keyframes::Keyframes()
	: m_functor(Linear)
{

}

void Keyframes::insert(const Keyframe& keyframe)
{
	m_keyframes.insert(keyframe);
}

vmath::quaternion Keyframes::calculateInterpolation(GMfloat percentage, bool normalize)
{
	std::pair<const Keyframe*, const Keyframe*> lp = findLowerAndUpper(percentage);
	LOG_ASSERT_MSG(lp.first && lp.second, "Missing '1' in animation keyframe percentage or missing animationduration");

	vmath::quaternion ret = interpolation(percentage, lp.first, lp.first->movement, lp.second, lp.second->movement, getFunctor());
	if (normalize)
	{
		vmath::vec3 dir = vmath::normalize(vmath::vec3(ret[0], ret[1], ret[2]));
		return vmath::quaternion(dir[0], dir[1], dir[2], ret[3]);
	}
	return vmath::quaternion(ret[0], ret[1], ret[2], ret[3]);
}

void Keyframes::setFunctor(Interpolation functor)
{
	m_functor = functor;
}

bool Keyframes::isEmpty()
{
	return m_keyframes.size() == 0;
}

std::pair<const Keyframe*, const Keyframe*> Keyframes::findLowerAndUpper(GMfloat percentage)
{
	const Keyframe* first = nullptr;
	const Keyframe* second = nullptr;
	for (auto iter = m_keyframes.begin(); iter != m_keyframes.end(); iter++)
	{
		const Keyframe& keyframe = (*iter);
		if (percentage >= keyframe.percentage)
			first = &keyframe;
		if (!second && percentage <= keyframe.percentage)
			second = &keyframe;
	}
	return std::make_pair(first, second);
}

InterpolationFunctor Keyframes::getFunctor()
{
	switch (m_functor)
	{
	case Keyframes::Linear:
		return linearFunctor;
	default:
		ASSERT(false);
		break;
	}
	return nullptr;
}