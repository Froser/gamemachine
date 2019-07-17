#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include <gmcommon.h>
#include "gmphysicsobject.h"
BEGIN_NS

class GMGameWorld;
class GMPhysicsObject;
GM_PRIVATE_OBJECT(GMPhysicsWorld)
{
	GMGameWorld* world;
	GMfloat gravity;
	bool enabled = true;
};

class GM_EXPORT GMPhysicsWorld : public GMObject
{
	GM_DECLARE_PRIVATE(GMPhysicsWorld)
	GM_DECLARE_PROPERTY(Enabled, enabled)

public:
	GMPhysicsWorld(GMGameWorld* world);
	virtual ~GMPhysicsWorld();

public:
	virtual void update(GMDuration dt, GMGameObject* obj) = 0;
	virtual void applyMove(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) {}
	virtual void applyJump(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) {}
};

END_NS
#endif