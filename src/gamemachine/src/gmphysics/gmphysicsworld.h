#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include <gmcommon.h>
#include "gmphysicsstructs.h"
BEGIN_NS

class GMGameWorld;
class GMPhysicsObject;
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
	virtual void applyMove(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) {}
	virtual void applyJump(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) {}

public:
	virtual GMPhysicsObject* createPhysicsObject();
};

END_NS
#endif