#include "stdafx.h"
#include "algorithm.h"
#include "gmdatacore/object.h"

const GMfloat PI2 = 6.283185306f;
const GMfloat PIPI2 = 19.73920879476716;

struct Point
{
	GMfloat x, y, z;
};

typedef Point __vector;

static Point getNormal(GMfloat u, GMfloat v)
{
	GMfloat x = sin(PI*v)*cos(PI2*u);
	GMfloat y = sin(PI*v)*sin(PI2*u);
	GMfloat z = cos(PI*v);
	Point p = { x, y, z };
	return p;
}

static void pushPoint(std::vector<GMfloat>& vertices, std::vector<GMfloat>& normals, const Point& normal, GMfloat radius)
{
	vertices.push_back(normal.x * radius);
	vertices.push_back(normal.y * radius);
	vertices.push_back(normal.z * radius);
	vertices.push_back(1.0f);
	normals.push_back(normal.x);
	normals.push_back(normal.y);
	normals.push_back(normal.z);
	normals.push_back(1.0f);
}