#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include <gmcommon.h>
#include "gmphysicsstructs.h"
BEGIN_NS

class GMGameWorld;
struct GMPhysicsObject;
GM_PRIVATE_OBJECT(GMPhysicsWorld)
{
	GMGameWorld* world;
	GMfloat gravity;
};

class GMPhysicsWorld : public GMObject
{
	DECLARE_PRIVATE(GMPhysicsWorld)

public:
	GMPhysicsWorld(GMGameWorld* world);
	virtual ~GMPhysicsWorld();

public:
	virtual void simulate(GMGameObject* obj) = 0;
	virtual void applyMove(const GMPhysicsObject& phy, const GMPhysicsMoveArgs& args) {}
	virtual void applyJump(const GMPhysicsObject& phy) {}
};

END_NS
#endif