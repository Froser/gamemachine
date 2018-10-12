#include "stdafx.h"
#include "gmdiscretedynamicsworld.h"
#include "gmbulletincludes.h"
#include "gmdata/gmmodel.h"
#include "gmengine/gmgameworld.h"
#include "gmconstraint.h"
#include <algorithm>

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
	GM_delete(d->bulletRigidPool);
}

void GMDiscreteDynamicsWorld::update(GMDuration dt, GMGameObject* obj)
{
	D(d);
	D_BASE(db, Base);
	GM_ASSERT(d->worldImpl);
	d->worldImpl->stepSimulation(dt);
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
		gameObject->setTranslation(Translate(GMVec3(pos[0], pos[1], pos[2])));
		gameObject->setRotation(GMQuat(rotation[0], rotation[1], rotation[2], rotation[3]));
	}
}

void GMDiscreteDynamicsWorld::setGravity(const GMVec3& gravity)
{
	D(d);
	GMFloat4 f4;
	gravity.loadFloat4(f4);
	d->worldImpl->setGravity(btVector3(f4[0], f4[1], f4[2]));
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

GMPhysicsRayTestResult GMDiscreteDynamicsWorld::rayTest(const GMVec3& rayFromWorld, const GMVec3& rayToWorld)
{
	D(d);
	GMPhysicsRayTestResult result;
	result.rayFromWorld = rayFromWorld;
	result.rayToWorld = rayToWorld;

	GMFloat4 f4_rayFromWorld, f4_rayToWorld;
	rayFromWorld.loadFloat4(f4_rayFromWorld);
	rayToWorld.loadFloat4(f4_rayToWorld);

	btVector3 rfw(f4_rayFromWorld[0], f4_rayFromWorld[1], f4_rayFromWorld[2]),
		rtw(f4_rayToWorld[0], f4_rayToWorld[1], f4_rayToWorld[2]);
	btCollisionWorld::ClosestRayResultCallback rayCallback(rfw, rtw);
	d->worldImpl->rayTest(rfw, rtw, rayCallback);
	if (rayCallback.hasHit())
	{
		result.hit = true;
		result.hitPointWorld = GMVec3(rayCallback.m_hitPointWorld[0], rayCallback.m_hitPointWorld[1], rayCallback.m_hitPointWorld[2]);
		result.hitNormalWorld = GMVec3(rayCallback.m_hitNormalWorld[0], rayCallback.m_hitNormalWorld[1], rayCallback.m_hitNormalWorld[2]);
		result.hitObject = static_cast<GMRigidPhysicsObject*>(rayCallback.m_collisionObject->getUserPointer());
	}

	return result;
}