#ifndef __ALGORITHMN_H__
#define __ALGORITHMN_H__
#include "common.h"
#include "LinearMath\btAlignedObjectArray.h"
#include "LinearMath\btVector3.h"
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

struct Geometry
{
	static bool isPointInsidePlanes(const btAlignedObjectArray<btVector3>& planeEquations, const btVector3& point, btScalar margin);
	static void getVerticesFromPlaneEquations(const btAlignedObjectArray<btVector3>& planeEquations, btAlignedObjectArray<btVector3>& verticesOut, UpAxis up = Y_AXIS);
};

inline btVector3 makeVector(GMfloat x, GMfloat y, GMfloat z, UpAxis up = Y_AXIS)
{
	return up == Y_AXIS ? btVector3(x, y, z) : btVector3(x, z, y);
}

END_NS
#endif