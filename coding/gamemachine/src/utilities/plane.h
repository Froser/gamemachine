#ifndef __PLANE_H__
#define __PLANE_H__
#include "common.h"
#include "LinearMath\btVector3.h"
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
	Plane() : normal(btVector3(0.0f, 0.0f, 0.0f)), intercept(0.0f)
	{}
	Plane(const btVector3& newNormal, float newIntercept) : normal(newNormal), intercept(newIntercept)
	{}
	Plane(const Plane & rhs)
	{
		normal = rhs.normal;	intercept = rhs.intercept;
	}

	~Plane() {}

	void setNormal(const btVector3 & rhs) { normal = rhs; }
	void setIntercept(float newIntercept) { intercept = newIntercept; }
	void setFromPoints(const btVector3 & p0, const btVector3 & p1, const btVector3 & p2);

	void calculateIntercept(const btVector3 & pointOnPlane) { intercept = -normal.dot(pointOnPlane); }

	void normalize(void);

	btVector3 getNormal() { return normal; }
	float getIntercept() { return intercept; }

	//find point of intersection of 3 planes
	bool intersect3(const Plane & p2, const Plane & p3, btVector3 & result);

	float getDistance(const btVector3 & point) const;
	PointPosition classifyPoint(const btVector3 & point) const;

	Plane lerp(const Plane & p2, float factor);

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
	btVector3 normal;	//X.N+intercept=0
	float intercept;
};

END_NS
#endif