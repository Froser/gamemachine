#include "stdafx.h"
#include "plane.h"

#define EPSILON 0.01f

void Plane::setFromPoints(const btVector3 & p0, const btVector3 & p1, const btVector3 & p2)
{
	normal = (p1 - p0).cross(p2 - p0);

	normal.normalize();

	calculateIntercept(p0);
}

void Plane::normalize()
{
	float normalLength = normal.length();
	normal /= normalLength;
	intercept /= normalLength;
}

bool Plane::intersect3(const Plane & p2, const Plane & p3, btVector3 & result)//find point of intersection of 3 planes
{
	float denominator = normal.dot((p2.normal).cross(p3.normal));
	//scalar triple product of normals
	if (denominator == 0.0f)									//if zero
		return false;										//no intersection

	btVector3 temp1, temp2, temp3;
	temp1 = (p2.normal.cross(p3.normal))*intercept;
	temp2 = (p3.normal.cross(normal))*p2.intercept;
	temp3 = (normal.cross(p2.normal))*p3.intercept;

	result = (temp1 + temp2 + temp3) / (-denominator);

	return true;
}

float Plane::getDistance(const btVector3 & point) const
{
	return point.x() *normal.x() + point.y() *normal.y() + point.z() *normal.z() + intercept;
}

PointPosition Plane::classifyPoint(const btVector3 & point) const
{
	float distance = point.x() *normal.x() + point.y() *normal.y() + point.z() *normal.z() + intercept;

	if (distance > EPSILON)	//==0.0f is too exact, give a bit of room
		return POINT_IN_FRONT_OF_PLANE;

	if (distance < -EPSILON)
		return POINT_BEHIND_PLANE;

	return POINT_ON_PLANE;	//otherwise
}

Plane Plane::lerp(const Plane & p2, float factor)
{
	Plane result;
	result.normal = normal*(1.0f - factor) + p2.normal*factor;
	result.normal.normalize();

	result.intercept = intercept*(1.0f - factor) + p2.intercept*factor;

	return result;
}

bool Plane::operator ==(const Plane & rhs) const
{
	if (normal == rhs.normal && intercept == rhs.intercept)
		return true;

	return false;
}
