#include "stdafx.h"
#include "gmdiscretedynamicsworld.h"
#include "gmbulletincludes.h"
#include "gmbullethelper.h"
#include "gmdata/gmmodel.h"
#include "gmengine/gmgameworld.h"

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

GMDiscreteDynamicsWorld::~GMDiscreteDynamicsWorld()
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

void GMDiscreteDynamicsWorld::simulate(GMGameObject* obj)
{
	D(d);
	D_BASE(db, Base);
	GM_ASSERT(d->worldImpl);
	d->worldImpl->stepSimulation(GM.getGameMachineRunningStates().lastFrameElpased);
	syncTransform();
}

void GMDiscreteDynamicsWorld::syncTransform()
{
	D(d);
	for (auto& rigid : d->rigidObjs)
	{
		btRigidBody* body = rigid->getRigidBody();
		const btTransform& transform = body->getWorldTransform();
		btVector3 pos = transform.getOrigin();
		btQuaternion rotation = transform.getRotation();

		GMGameObject* gameObject = rigid->getGameObject();
		gameObject->setTranslation(glm::translate(glm::vec3(pos[0], pos[1], pos[2])));
		gameObject->setRotation(glm::quat(rotation[0], rotation[1], rotation[2], rotation[3]));
	}
}

void GMDiscreteDynamicsWorld::setGravity(const glm::vec3& gravity)
{
	D(d);
	d->worldImpl->setGravity(btVector3(gravity[0], gravity[1], gravity[2]));
}

void GMDiscreteDynamicsWorld::addRigidObject(AUTORELEASE GMRigidPhysicsObject* rigidObj)
{
	D(d);
	D_BASE(db, Base);
	d->rigidObjs.push_back(rigidObj);
	d->worldImpl->addRigidBody(rigidObj->getRigidBody());
}