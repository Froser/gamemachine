#include "stdafx.h"
#include "gmbullethelper.h"
#include "gmbulletincludes.h"

void GMBulletHelper::collisionShape2TriangleMesh(
	btCollisionShape* collisionShape,
	const btTransform& parentTransform,
	btAlignedObjectArray<btVector3>& vertexPositions,
	btAlignedObjectArray<btVector3>& vertexNormals,
	btAlignedObjectArray<GMint>& indicesOut)
{
//	switch (collisionShape->getShapeType())
//	{
//	default:
//	{
		if (collisionShape->isConvex())
		{
			btConvexShape* convex = static_cast<btConvexShape*>(collisionShape);
			btShapeHull* hull = new btShapeHull(convex);
			hull->buildHull(0.0);
			for (GMint t = 0; t < hull->numTriangles(); t++)
			{
				btVector3 triNormal;
				GMint index0 = hull->getIndexPointer()[t * 3 + 0];
				GMint index1 = hull->getIndexPointer()[t * 3 + 1];
				GMint index2 = hull->getIndexPointer()[t * 3 + 2];
				btVector3 pos0 = parentTransform*hull->getVertexPointer()[index0];
				btVector3 pos1 = parentTransform*hull->getVertexPointer()[index1];
				btVector3 pos2 = parentTransform*hull->getVertexPointer()[index2];
				triNormal = (pos1 - pos0).cross(pos2 - pos0);
				triNormal.normalize();

				for (GMint v = 0; v < 3; v++)
				{
					GMint index = hull->getIndexPointer()[t * 3 + v];
					btVector3 pos = parentTransform*hull->getVertexPointer()[index];
					indicesOut.push_back(vertexPositions.size());
					vertexPositions.push_back(pos);
					vertexNormals.push_back(triNormal);
				}
			}
			GM_delete(hull);
		}
//	}
//	}
}