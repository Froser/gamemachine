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

bool Geometry::isPointInsidePlanes(const btAlignedObjectArray<btVector3>& planeEquations, const btVector3& point, btScalar	margin)
{
	int numbrushes = planeEquations.size();
	for (int i = 0; i < numbrushes; i++)
	{
		const btVector3& N1 = planeEquations[i];
		btScalar dist = btScalar(N1.dot(point)) + btScalar(N1[3]) - margin;
		if (dist > btScalar(0.))
		{
			return false;
		}
	}
	return true;
}


void Geometry::getVerticesFromPlaneEquations(const btAlignedObjectArray<btVector3>& planeEquations, btAlignedObjectArray<btVector3>& verticesOut, UpAxis up)
{
	const int numbrushes = planeEquations.size();
	// brute force:
	for (int i = 0; i < numbrushes; i++)
	{
		const btVector3& N1 = planeEquations[i];


		for (int j = i + 1; j < numbrushes; j++)
		{
			const btVector3& N2 = planeEquations[j];

			for (int k = j + 1; k < numbrushes; k++)
			{

				const btVector3& N3 = planeEquations[k];

				btVector3 n2n3; n2n3 = N2.cross(N3);
				btVector3 n3n1; n3n1 = N3.cross(N1);
				btVector3 n1n2; n1n2 = N1.cross(N2);

				if ((n2n3.length2() > btScalar(0.0001)) &&
					(n3n1.length2() > btScalar(0.0001)) &&
					(n1n2.length2() > btScalar(0.0001)))
				{
					//point P out of 3 plane equations:

					//	d1 ( N2 * N3 ) + d2 ( N3 * N1 ) + d3 ( N1 * N2 )  
					//P =  -------------------------------------------------------------------------  
					//   N1 . ( N2 * N3 )  


					btScalar quotient = (N1.dot(n2n3));
					if (btFabs(quotient) > btScalar(0.000001))
					{
						quotient = btScalar(-1.) / quotient;
						n2n3 *= N1[3];
						n3n1 *= N2[3];
						n1n2 *= N3[3];
						btVector3 potentialVertex = n2n3;
						potentialVertex += n3n1;
						potentialVertex += n1n2;
						potentialVertex *= quotient;

						//check if inside, and replace supportingVertexOut if needed
						if (isPointInsidePlanes(planeEquations, potentialVertex, btScalar(0.01)))
						{
							if (up == Z_AXIS)
							{
								//swap y and z
								GMfloat t = potentialVertex[1];
								potentialVertex[1] = potentialVertex[2];
								potentialVertex[2] = t;
							}

							verticesOut.push_back(potentialVertex);
						}
					}
				}
			}
		}
	}
}