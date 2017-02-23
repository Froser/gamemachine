#ifndef __PLANE_H__
#define __PLANE_H__
#include "common.h"
#include "vmath.h"
BEGIN_NS

enum PointPosition
{
	POINT_ON_PLANE = 0,
	POINT_IN_FRONT_OF_PLANE,
	POINT_BEHIND_PLANE,
};

class Plane
{
public:
	Plane() : normal(vmath::vec3(0.0f, 0.0f, 0.0f)), intercept(0.0f)
	{}
	Plane(const vmath::vec3& newNormal, GMfloat newIntercept) : normal(newNormal), intercept(newIntercept)
	{}
	Plane(const Plane & rhs)
	{
		normal = rhs.normal;	intercept = rhs.intercept;
	}

	~Plane() {}

	void setNormal(const vmath::vec3 & rhs) { normal = rhs; }
	void setIntercept(GMfloat newIntercept) { intercept = newIntercept; }
	void setFromPoints(const vmath::vec3 & p0, const vmath::vec3 & p1, const vmath::vec3 & p2);

	void calculateIntercept(const vmath::vec3 & pointOnPlane) { intercept = -vmath::dot(normal ,pointOnPlane); }

	void normalize(void);

	vmath::vec3 getNormal() { return normal; }
	GMfloat getIntercept() { return intercept; }

	//find point of intersection of 3 planes
	bool intersect3(const Plane & p2, const Plane & p3, vmath::vec3 & result);

	GMfloat getDistance(const vmath::vec3 & point) const;
	PointPosition classifyPoint(const vmath::vec3 & point) const;

	Plane lerp(const Plane & p2, GMfloat factor);

	//operators
	bool operator==(const Plane & rhs) const;
	bool operator!=(const Plane & rhs) const
	{
		return!((*this) == rhs);
	}

	//unary operators
	Plane operator-(void) const { return Plane(-normal, -intercept); }
	Plane operator+(void) const { return (*this); }

	//member variables
	vmath::vec3 normal;	//X.N+intercept=0
	GMfloat intercept;
};

END_NS
#endif