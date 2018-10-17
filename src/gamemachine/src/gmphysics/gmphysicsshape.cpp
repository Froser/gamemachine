#include "stdafx.h"
#include "gmphysicsshape.h"
#include "gmbulletincludes.h"
#include <linearmath.h>
#include <gmmodel.h>
#include <gmassets.h>

namespace
{
	void collisionShape2TriangleMesh(
		btCollisionShape* collisionShape,
		const btTransform& parentTransform,
		btAlignedObjectArray<btVector3>& vertexPositions,
		btAlignedObjectArray<btVector3>& vertexNormals,
		btAlignedObjectArray<GMint32>& indicesOut)
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
			for (GMint32 t = 0; t < hull->numTriangles(); t++)
			{
				btVector3 triNormal;
				GMint32 index0 = hull->getIndexPointer()[t * 3 + 0];
				GMint32 index1 = hull->getIndexPointer()[t * 3 + 1];
				GMint32 index2 = hull->getIndexPointer()[t * 3 + 2];
				btVector3 pos0 = parentTransform*hull->getVertexPointer()[index0];
				btVector3 pos1 = parentTransform*hull->getVertexPointer()[index1];
				btVector3 pos2 = parentTransform*hull->getVertexPointer()[index2];
				triNormal = (pos1 - pos0).cross(pos2 - pos0);
				triNormal.normalize();

				for (GMint32 v = 0; v < 3; v++)
				{
					GMint32 index = hull->getIndexPointer()[t * 3 + v];
					btVector3 pos = parentTransform*hull->getVertexPointer()[index];
					indicesOut.push_back(vertexPositions.size());
					vertexPositions.push_back(pos);
					vertexNormals.push_back(triNormal);
				}
			}
			GM_delete(hull);
		}
		else if (collisionShape->isCompound())
		{
			btCompoundShape* compound = gm_cast<btCompoundShape*>(collisionShape);
			for (int i = 0; i < compound->getNumChildShapes(); i++)
			{

				btTransform childWorldTrans = parentTransform * compound->getChildTransform(i);
				collisionShape2TriangleMesh(compound->getChildShape(i), childWorldTrans, vertexPositions, vertexNormals, indicesOut);
			}
		}
		else
		{
			GM_ASSERT(false);
		}
		//	}
		//	}
	}

	void createModelFromCollisionShape(
		btCollisionShape* shape,
		OUT GMModel** model)
	{
		if (model)
		{
			GMModel* out = *model = new GMModel();

			btTransform origin;
			origin.setIdentity();

			btAlignedObjectArray<btVector3> vertexPositions;
			btAlignedObjectArray<btVector3> vertexNormals;
			btAlignedObjectArray<GMint32> indices;
			collisionShape2TriangleMesh(shape, origin, vertexPositions, vertexNormals, indices);

			GMMesh* body = new GMMesh(out);
			out->getShader().setCull(GMS_Cull::None);
			GMint32 faceCount = indices.size() / 3;
			for (GMint32 i = 0; i < faceCount; ++i)
			{
				for (GMint32 j = 0; j < 3; ++j)
				{
					GMint32 idx = i * 3 + j;
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

	void createConvexHullShape(GMModel* model, const GMVec3& scaling, bool optimizeConvex, OUT btConvexHullShape** shape)
	{
		if (shape)
		{
			btConvexHullShape* btShape = new btConvexHullShape();
			if (model->getDrawMode() == GMModelDrawMode::Vertex)
			{
				for (const auto& mesh : model->getMeshes())
				{
					for (const auto& v : mesh->vertices())
					{
						btShape->addPoint(btVector3(v.positions[0], v.positions[1], v.positions[2]), false);
					}
				}
			}
			else
			{
				for (const auto& mesh : model->getMeshes())
				{
					const auto& vertices = mesh->vertices();
					for (const auto& i : mesh->indices())
					{
						btShape->addPoint(btVector3(vertices[i].positions[0], vertices[i].positions[1], vertices[i].positions[2]), false);
					}
				}
			}

			btShape->recalcLocalAabb();
			btVector3 localScaling(scaling.getX(), scaling.getY(), scaling.getZ());
			btShape->setLocalScaling(localScaling);

			if (optimizeConvex)
				btShape->optimizeConvexHull();

			*shape = btShape;
		}
	}
}

GMPhysicsShape::~GMPhysicsShape()
{
	D(d);
	GM_delete(d->shape);
}

void GMPhysicsShape::setShape(AUTORELEASE btCollisionShape* shape)
{
	D(d);
	if (d->shape)
		GM_delete(d->shape);
	d->shape = shape;
}

btCollisionShape* GMPhysicsShape::getBulletShape()
{
	D(d);
	return d->shape;
}

const btCollisionShape* GMPhysicsShape::getBulletShape() const
{
	D(d);
	return d->shape;
}

//////////////////////////////////////////////////////////////////////////
// Helper
void GMPhysicsShapeHelper::createCubeShape(const GMVec3& halfExtents, REF GMPhysicsShapeAsset& physicsShape)
{
	GMFloat4 fs_halfExtents;
	halfExtents.loadFloat4(fs_halfExtents);
	GMPhysicsShape* shape = new GMPhysicsShape();
	shape->setShape(new btBoxShape(btVector3(fs_halfExtents[0], fs_halfExtents[1], fs_halfExtents[2])));
	physicsShape = GMAsset(GMAssetType::PhysicsShape, shape);
}

void GMPhysicsShapeHelper::createCylinderShape(const GMVec3& halfExtents, REF GMPhysicsShapeAsset& physicsShape)
{
	GMFloat4 fs_halfExtents;
	halfExtents.loadFloat4(fs_halfExtents);
	GMPhysicsShape* shape = new GMPhysicsShape();
	shape->setShape(new btCylinderShape(btVector3(fs_halfExtents[0], fs_halfExtents[1], fs_halfExtents[2])));
	physicsShape = GMAsset(GMAssetType::PhysicsShape, shape);
}

void GMPhysicsShapeHelper::createConeShape(GMfloat radius, GMfloat height, REF GMPhysicsShapeAsset& physicsShape)
{
	GMPhysicsShape* shape = new GMPhysicsShape();
	shape->setShape(new btConeShape(radius, height));
	physicsShape = GMAsset(GMAssetType::PhysicsShape, shape);
}

void GMPhysicsShapeHelper::createSphereShape(GMfloat radius, REF GMPhysicsShapeAsset& physicsShape)
{
	GMPhysicsShape* shape = new GMPhysicsShape();
	shape->setShape(new btSphereShape(radius));
	physicsShape = GMAsset(GMAssetType::PhysicsShape, shape);
}

bool GMPhysicsShapeHelper::createConvexShapeFromTriangleModel(
	GMModelAsset model,
	REF GMPhysicsShapeAsset& physicsShape,
	bool optimizeConvex,
	const GMVec3& scaling
)
{
	if (model.isEmpty())
		return false;

	GMModels* models = model.getModels();
	if (!models)
	{
		gm_warning(gm_dbg_wrap("not a valid models asset."));
		return false;
	}

	auto& vecModels = models->getModels();
	btCollisionShape* btShape = nullptr;
	if (vecModels.size() == 0)
	{
		gm_warning(gm_dbg_wrap("not a valid model asset."));
		return false;
	}
	else if (vecModels.size() == 1)
	{
		btConvexHullShape* s = nullptr;
		createConvexHullShape(vecModels.front().getModel(), scaling, optimizeConvex, &s);
		GM_ASSERT(s);
		btShape = s;
	}
	else
	{
		btCompoundShape* cs = new btCompoundShape();
		btTransform identityTrans;
		identityTrans.setIdentity();
		for (auto& model : vecModels)
		{
			btConvexHullShape* s = nullptr;
			createConvexHullShape(vecModels.front().getModel(), scaling, optimizeConvex, &s);
			GM_ASSERT(s);
			cs->addChildShape(identityTrans, s);
		}
		btShape = cs;
	}

	GMPhysicsShape* shape = new GMPhysicsShape();
	shape->setShape(btShape);
	physicsShape = GMAsset(GMAssetType::PhysicsShape, shape);
	return true;
}

void GMPhysicsShapeHelper::createModelFromShape(
	GMPhysicsShape* shape,
	REF GMModelAsset& asset
)
{
	GMModelAsset& cache = shape->getModelCache();
	if (!cache.getModel())
	{
		GMModel* model = nullptr;
		createModelFromCollisionShape(shape->getBulletShape(), &model);
		GM_ASSERT(model);
		asset = GMAsset(GMAssetType::Model, model);
		shape->setModelCache(asset);
	}
	else
	{
		// Model already exists
		GMModel* newModel = new GMModel(cache);
		asset = GMAsset(GMAssetType::Model, newModel);
	}
}