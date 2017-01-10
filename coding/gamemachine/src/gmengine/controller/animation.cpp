#include "stdafx.h"
#include "animation.h"
#include "utilities\assert.h"

static GMfloat linearFunctor(const __vector& v1, const __vector& v2, GMfloat x)
{
	ASSERT(v2.x != v1.x);
	return v1.y + (v2.y - v1.y) * (x - v1.x) / (v2.x - v1.x);
}

static vmath::quaternion interpolation(GMfloat v,
	const Keyframe* lower, 
	const KeyframeMovement& lowerMovement, 
	const Keyframe* upper, 
	const KeyframeMovement& upperMovement,
	InterpolationFunctor functor)
{
	GMfloat l = lower->percentage, u = upper->percentage;

	GMfloat x = functor(__vector(l, lowerMovement.direction[0]), __vector(u, upperMovement.direction[0]), v);
	GMfloat y = functor(__vector(l, lowerMovement.direction[1]), __vector(u, upperMovement.direction[1]), v);
	GMfloat z = functor(__vector(l, lowerMovement.direction[2]), __vector(u, upperMovement.direction[2]), v);
	GMfloat a = functor(__vector(l, lowerMovement.value), __vector(u, upperMovement.value), v);

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

vmath::quaternion Keyframes::calculateRotation(GMfloat percentage)
{
	std::pair<const Keyframe*, const Keyframe*> lp = findLowerAndUpper(percentage);
	LOG_ASSERT_MSG(lp.first && lp.second, "Missing '1' in animation keyframe percentage");

	vmath::quaternion ret = interpolation(percentage, lp.first, lp.first->movement, lp.second, lp.second->movement, getFunctor());
	vmath::vec3 dir = vmath::normalize(vmath::vec3(ret[0], ret[1], ret[2]));
	return vmath::quaternion(dir[0], dir[1], dir[2], ret[3]);
}

void Keyframes::setFunctor(Interpolation functor)
{
	m_functor = functor;
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
		if (!second && percentage < keyframe.percentage)
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