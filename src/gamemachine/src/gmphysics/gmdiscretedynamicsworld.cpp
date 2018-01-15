#include "stdafx.h"
#include "gmdiscretedynamicsworld.h"
#include "gmbulletincludes.h"

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
	d->rigidObjs.push_back(rigidObj);
	d->worldImpl->addRigidBody(rigidObj->getRigidBody());
}