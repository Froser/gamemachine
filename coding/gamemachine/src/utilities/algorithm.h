#ifndef __ALGORITHMN_H__
#define __ALGORITHMN_H__
#include "common.h"
BEGIN_NS

class ChildObject;
class Algorithm
{
public:
	static void createSphere(GMfloat radius, GMint slices, GLint stacks, OUT ChildObject** obj);
};

// interpolations
struct InterpolationVector2
{
	InterpolationVector2(GMfloat _x, GMfloat _y)
		: x(_x)
		, y(_y)
	{
	}

	GMfloat x, y;
};
typedef GMfloat(*InterpolationFunctor) (const InterpolationVector2& v1, const InterpolationVector2& v2, GMfloat x);

static GMfloat linearFunctor(const InterpolationVector2& v1, const InterpolationVector2& v2, GMfloat x)
{
	if (v2.x == v1.x)
		return v1.y;
	return v1.y + (v2.y - v1.y) * (x - v1.x) / (v2.x - v1.x);
}

END_NS
#endif