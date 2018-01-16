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
};

class GMDiscreteDynamicsWorld : public GMPhysicsWorld
{
	DECLARE_PRIVATE_AND_BASE(GMDiscreteDynamicsWorld, GMPhysicsWorld);

public:
	GMDiscreteDynamicsWorld(GMGameWorld* world);

public:
	void setGravity(const glm::vec3& gravity);
	void addRigidObjects(AUTORELEASE GMRigidPhysicsObject* rigidObj);

public:
	virtual void simulate(GMGameObject* obj) override;
};

END_NS
#endif