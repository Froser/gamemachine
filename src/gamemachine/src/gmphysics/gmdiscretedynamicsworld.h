#ifndef __GMDISCRETEDYNAMICSWORLD_H__
#define __GMDISCRETEDYNAMICSWORLD_H__
#include <gmcommon.h>
#include "gmphysicsworld.h"
#include "gmbulletforward.h"

BEGIN_NS

GM_PRIVATE_OBJECT(GMDiscreteDynamicsWorld)
{
	btDiscreteDynamicsWorld* worldImpl = nullptr;
	btDefaultCollisionConfiguration* collisionConfiguration = nullptr;
	btCollisionDispatcher* dispatcher = nullptr;
	btBroadphaseInterface* overlappingPairCache = nullptr;
	btSequentialImpulseConstraintSolver* solver = nullptr;
	Vector<GMRigidPhysicsObject*> rigidObjs;
	Vector<AUTORELEASE btRigidBody*> bulletRigidPool;
};

class GMDiscreteDynamicsWorld : public GMPhysicsWorld
{
	DECLARE_PRIVATE_AND_BASE(GMDiscreteDynamicsWorld, GMPhysicsWorld);

public:
	GMDiscreteDynamicsWorld(GMGameWorld* world);
	~GMDiscreteDynamicsWorld();

public:
	void setGravity(const glm::vec3& gravity);
	void addRigidObject(AUTORELEASE GMRigidPhysicsObject* rigidObj);
	GMPhysicsRayTestResult rayTest(const glm::vec3& rayFromWorld, const glm::vec3& rayToWorld);

public:
	virtual void simulate(GMGameObject* obj) override;

private:
	void syncTransform();
};

END_NS
#endif