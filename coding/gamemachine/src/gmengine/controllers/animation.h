#ifndef __ANIMATION_H__
#define __ANIMATION_H__
#include "common.h"
#include <set>
#include <vector>
#include "utilities/vmath.h"
#include "utilities/algorithm.h"
BEGIN_NS

struct KeyframeMovement
{
	GMfloat direction[3];
	GMfloat value;
};

struct Keyframe
{
	GMfloat percentage;
	KeyframeMovement movement;
};

struct Keyframe_LESS
{
	bool operator () (const Keyframe& l, const Keyframe& r)
	{
		return l.percentage < r.percentage;
	}
};

class Keyframes
{
public:
	Keyframes();

	enum Interpolation
	{
		Linear,
	};

public:
	void insert(const Keyframe& keyframe);
	vmath::quaternion calculateInterpolation(GMfloat percentage, bool normalize);
	void setFunctor(Interpolation functor);
	bool isEmpty();

private:
	std::pair<const Keyframe*, const Keyframe*> findLowerAndUpper(GMfloat percentage);
	InterpolationFunctor getFunctor();

private:
	std::set<Keyframe, Keyframe_LESS> m_keyframes;
	Interpolation m_functor;
};

END_NS
#endif
