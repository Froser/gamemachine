#include "stdafx.h"
#include "gmbullethelper.h"
#include "gmbulletincludes.h"
#include <gmmodel.h>
#include <gmassets.h>
#include <gmgameobject.h>
#include <gmgameworld.h>
namespace
{
	void collisionShape2TriangleMesh(
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


	void createModelFromCollisionShape(
		btCollisionShape* shape,
		OUT GMModel** model)
	{
		GMModel* out = *model = new GMModel();

		btTransform origin;
		origin.setIdentity();

		btAlignedObjectArray<btVector3> vertexPositions;
		btAlignedObjectArray<btVector3> vertexNormals;
		btAlignedObjectArray<GMint> indices;
		collisionShape2TriangleMesh(shape, origin, vertexPositions, vertexNormals, indices);

		GMMesh* body = new GMMesh(out);
		out->getShader().setCull(GMS_Cull::None);
		GMint faceCount = indices.size() / 3;
		for (GMint i = 0; i < faceCount; ++i)
		{
			for (GMint j = 0; j < 3; ++j)
			{
				GMint idx = i * 3 + j;
				const btVector3& vertex = vertexPositions[indices[idx]];
				const btVector3& normal = vertexNormals[indices[idx]];
				GMVertex v = {
					{ vertex[0], vertex[1], vertex[2] },
					{ normal[0], normal[1], normal[2] }
				};
				body->vertex(v);
			}
		}
	}
}

void GMBulletHelper::createModelFromShape(
	GMPhysicsShape* shape,
	OUT GMModel** model)
{
	btCollisionShape* cs = shape->getBulletShape();
	if (!cs->getUserPointer())
	{
		createModelFromCollisionShape(cs, model);
		cs->setUserPointer(*model); // Save the model. DO NOT delete this model object when you need create more
	}
	else
	{
		// Model already exists
		GMModel* modelCache = static_cast<GMModel*>(cs->getUserPointer());
		*model = new GMModel(*modelCache); //use same vertex array
	}
}