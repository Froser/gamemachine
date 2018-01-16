#include "stdafx.h"
#include "gmdiscretedynamicsworld.h"
#include "gmbulletincludes.h"
#include "gmbullethelper.h"
#include "gmdata/gmmodel.h"
#include "gmengine/gmgameworld.h"

namespace
{
	void createModelFromCollisionShape(btCollisionShape* shape, const glm::vec3& color, OUT GMModel** model)
	{
		GMModel* out = *model = new GMModel();
		
		btTransform origin;
		origin.setIdentity();

		btAlignedObjectArray<btVector3> vertexPositions;
		btAlignedObjectArray<btVector3> vertexNormals;
		btAlignedObjectArray<GMint> indices;
		GMBulletHelper::collisionShape2TriangleMesh(shape, origin, vertexPositions, vertexNormals, indices);

		GMMesh* body = out->getMesh();
		GMComponent* component = new GMComponent(body);
		component->getShader().getMaterial().ka = color;
		component->getShader().setCull(GMS_Cull::NONE);
		GMint faceCount = indices.size() / 3;
		for (GMint i = 0; i < faceCount; ++i)
		{
			component->beginFace();
			for (GMint j = 0; j < 3; ++j)
			{
				GMint idx = i * 3 + j;
				const btVector3& vertex = vertexPositions[indices[idx]];
				const btVector3& normal = vertexNormals[indices[idx]];
				component->vertex(vertex[0], vertex[1], vertex[2]);
				component->normal(normal[0], normal[1], normal[2]);
			}
			component->endFace();
		}
	}
}

GMDiscreteDynamicsWorld::GMDiscreteDynamicsWorld(GMGameWorld* world)
	: GMPhysicsWorld(world)
{
	D(d);
	d->collisionConfiguration = new btDefaultCollisionConfiguration();
	d->dispatcher = new btCollisionDispatcher(d->collisionConfiguration);
	d->overlappingPairCache = new btDbvtBroadphase();
	d->solver = new btSequentialImpulseConstraintSolver();
	d->worldImpl = new btDiscreteDynamicsWorld(d->dispatcher, d->overlappingPairCache, d->solver, d->collisionConfiguration);
}

void GMDiscreteDynamicsWorld::simulate(GMGameObject* obj)
{
	D(d);
	GM_delete(d->collisionConfiguration);
	GM_delete(d->dispatcher);
	GM_delete(d->overlappingPairCache);
	GM_delete(d->solver);
	GM_delete(d->worldImpl);

	for (auto& obj : d->rigidObjs)
	{
		GM_delete(obj);
	}
}

void GMDiscreteDynamicsWorld::setGravity(const glm::vec3& gravity)
{
	D(d);
	d->worldImpl->setGravity(btVector3(gravity[0], gravity[1], gravity[2]));
}

void GMDiscreteDynamicsWorld::addRigidObjects(AUTORELEASE GMRigidPhysicsObject* rigidObj)
{
	D(d);
	D_BASE(db, Base);
	d->rigidObjs.push_back(rigidObj);
	d->worldImpl->addRigidBody(rigidObj->getRigidBody());

	// Create mesh
	btCollisionShape* shape = rigidObj->getShape()->getBulletShape();
	GMAsset asset;
	if (shape->getUserPointer())
	{
		asset.asset = static_cast<GMModel*>(shape->getUserPointer());
		asset.type = GMAssetType::Model;
	}
	else
	{
		GMModel* model = nullptr;
		createModelFromCollisionShape(shape, glm::vec3(1, 0, 0), &model);
		asset = db->world->getAssets().insertAsset(GMAssetType::Model, model);
		shape->setUserPointer(asset.asset);
	}
	GM_ASSERT(asset.asset);
	rigidObj->getGameObject()->setModel(asset);
}