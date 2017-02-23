#include "stdafx.h"
#include "plane.h"

#define EPSILON 0.01f

void Plane::setFromPoints(const vmath::vec3 & p0, const vmath::vec3 & p1, const vmath::vec3 & p2)
{
	
	normal = vmath::cross((p1 - p0), (p2 - p0));

	normal = vmath::normalize(normal);

	calculateIntercept(p0);
}

void Plane::normalize()
{
	float normalLength = vmath::length(normal);
	normal /= normalLength;
	intercept /= normalLength;
}

bool Plane::intersect3(const Plane & p2, const Plane & p3, vmath::vec3 & result)//find point of intersection of 3 planes
{
	float denominator = vmath::dot(normal, ( vmath::cross(p2.normal, p3.normal) ));
	//scalar triple product of normals
	if (denominator == 0.0f)									//if zero
		return false;										//no intersection

	vmath::vec3 temp1, temp2, temp3;
	temp1 = (vmath::cross(p2.normal, p3.normal))*intercept;
	temp2 = (vmath::cross(p3.normal, normal))*p2.intercept;
	temp3 = (vmath::cross(normal, p2.normal))*p3.intercept;

	result = (temp1 + temp2 + temp3) / (-denominator);

	return true;
}

GMfloat Plane::getDistance(const vmath::vec3 & point) const
{
	return point[0]*normal[0] + point[1] *normal[1] + point[2] *normal[2] + intercept;
}

PointPosition Plane::classifyPoint(const vmath::vec3 & point) const
{
	float distance = point[0] *normal[0] + point[1] *normal[1] + point[2] *normal[2] + intercept;

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
	result.normal = vmath::normalize(result.normal);

	result.intercept = intercept*(1.0f - factor) + p2.intercept*factor;

	return result;
}

bool Plane::operator ==(const Plane & rhs) const
{
	if (normal == rhs.normal && intercept == rhs.intercept)
		return true;

	return false;
}
