#include "stdafx.h"
#include "algorithm.h"
#include "gmdatacore/object.h"

const GMfloat PI2 = 6.283185306f;
const GMfloat PIPI2 = 19.73920879476716;

struct Point
{
	GMfloat x, y, z;
};

typedef Point Vector;

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

void Algorithm::createSphere(GMfloat radius, GMint slices, GLint stacks, OUT Object** obj)
{
	Object* sphere = nullptr;
	if (obj)
	{
		*obj = new Object;
		sphere = *obj;
	}

	const GMfloat uStep = 1.0f / slices;
	const GMfloat vStep = 1.0f / stacks;
	
	GMvertexoffset offset = 0;
	// Top
	{
		Component* c = new Component();
		c->setEdgeCountPerPolygon(3);
		c->setOffset(0);
		Point top = getNormal(0, 0);
		Point first = getNormal(0, vStep);
		GMfloat u = 0;
		for (GMuint i = 0; i < slices; i++, u += uStep)
		{
			Point next = getNormal(u + uStep, vStep);
			Point nextNormal = getNormal(u + uStep, vStep);
			pushPoint(sphere->vertices(), sphere->normals(), top, radius);
			pushPoint(sphere->vertices(), sphere->normals(), first, radius);
			pushPoint(sphere->vertices(), sphere->normals(), next, radius);
			first = next;
		}
		offset = 3 * slices;
		sphere->appendComponent(c, offset);
	}

	// Mid
	{
		Component* c = new Component();
		c->setEdgeCountPerPolygon(4);
		c->setOffset(offset);
		GMfloat u = 0, v = vStep;
	
		for (GMuint j = 0; j < stacks - 2; j++, v += vStep)
		{
			for (GMuint i = 0; i < slices; i++, u += uStep)
			{
				Point tl = getNormal(u, v);
				Point bl = getNormal(u, v + vStep);
				Point br = getNormal(u + uStep, v + vStep);
				Point tr = getNormal(u + uStep, v);
				pushPoint(sphere->vertices(), sphere->normals(), tl, radius);
				pushPoint(sphere->vertices(), sphere->normals(), bl, radius);
				pushPoint(sphere->vertices(), sphere->normals(), br, radius);
				pushPoint(sphere->vertices(), sphere->normals(), tr, radius);
			}
		}
	
		GMuint count = slices * (stacks - 2) * 4;
		offset += count;
		sphere->appendComponent(c, offset);
	}

	// Bottom
	{
		GMfloat v = 1 - vStep;
		Component* c = new Component();
		c->setEdgeCountPerPolygon(3);
		c->setOffset(offset);
		Point bottom = getNormal(0, 1);
		Point first = getNormal(0, v);
		GMfloat u = 0;
		for (GMuint i = 0; i < slices; i++, u += uStep)
		{
			Point next = getNormal(u + uStep, v);
			pushPoint(sphere->vertices(), sphere->normals(), first, radius);
			pushPoint(sphere->vertices(), sphere->normals(), bottom, radius);
			pushPoint(sphere->vertices(), sphere->normals(), next, radius);
			first = next;
		}
		sphere->appendComponent(c, 3 * slices);
	}
}