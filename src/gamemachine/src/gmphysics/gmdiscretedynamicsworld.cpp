#include "stdafx.h"
#include "gmdiscretedynamicsworld.h"
#include "gmbulletincludes.h"
#include "gmbullethelper.h"
#include "gmdata/gmmodel.h"
#include "gmengine/gmgameworld.h"
#include "gmconstraint.h"

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
	for (auto& constraint : d->constraintObjs)
	{
		d->worldImpl->removeConstraint(constraint->getConstraint());
		GM_delete(constraint);
	}

	GM_delete(d->worldImpl);
	GM_delete(d->solver);
	GM_delete(d->overlappingPairCache);
	GM_delete(d->dispatcher);
	GM_delete(d->collisionConfiguration);

	for (auto& rigid : d->bulletRigidPool)
	{
		GM_delete(rigid);
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
	btRigidBody* btBody = rigidObj->getRigidBody();
	d->bulletRigidPool.push_back(btBody);
	rigidObj->detachRigidBody();
	d->worldImpl->addRigidBody(btBody);
}

void GMDiscreteDynamicsWorld::addConstraint(AUTORELEASE GMConstraint* constraint, bool disableCollisionsBetweenLinkedBodies)
{
	D(d);
	d->constraintObjs.push_back(constraint);
	d->worldImpl->addConstraint(constraint->getConstraint(), disableCollisionsBetweenLinkedBodies);
}

void GMDiscreteDynamicsWorld::removeConstraint(GMConstraint* constraint)
{
	D(d);
	auto iter = std::find(d->constraintObjs.begin(), d->constraintObjs.end(), constraint);
	if (iter != d->constraintObjs.end())
		d->constraintObjs.erase(iter);
	d->worldImpl->removeConstraint(constraint->getConstraint());
}

GMPhysicsRayTestResult GMDiscreteDynamicsWorld::rayTest(const glm::vec3& rayFromWorld, const glm::vec3& rayToWorld)
{
	D(d);
	GMPhysicsRayTestResult result;
	result.rayFromWorld = rayFromWorld;
	result.rayToWorld = rayToWorld;

	btVector3 rfw(rayFromWorld[0], rayFromWorld[1], rayFromWorld[2]), 
		rtw(rayToWorld[0], rayToWorld[1], rayToWorld[2]);
	btCollisionWorld::ClosestRayResultCallback rayCallback(rfw, rtw);
	d->worldImpl->rayTest(rfw, rtw, rayCallback);
	if (rayCallback.hasHit())
	{
		result.hit = true;
		result.hitPointWorld = glm::vec3(rayCallback.m_hitPointWorld[0], rayCallback.m_hitPointWorld[1], rayCallback.m_hitPointWorld[2]);
		result.hitNormalWorld = glm::vec3(rayCallback.m_hitNormalWorld[0], rayCallback.m_hitNormalWorld[1], rayCallback.m_hitNormalWorld[2]);
		result.hitObject = static_cast<GMRigidPhysicsObject*>(rayCallback.m_collisionObject->getUserPointer());
	}

	return result;
}