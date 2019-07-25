#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include <gmcommon.h>
#include <gmphysicsworld.h>
#include <extensions/bsp/gmbspphysicsworld.h>
#include "gmbsptrace.h"
#include "gmbsppatch.h"
BEGIN_NS

class GMBSPGameWorld;
class GMBSPMove;

GM_PRIVATE_CLASS(GMBSPPhysicsWorld);
class GMBSPPhysicsWorld : public GMPhysicsWorld
{
	GM_DECLARE_PRIVATE(GMBSPPhysicsWorld)
	GM_DECLARE_BASE(GMPhysicsWorld)

public:
	GMBSPPhysicsWorld(GMGameWorld* world);
	~GMBSPPhysicsWorld();

public:
	virtual void update(GMDuration dt, GMGameObject* obj) override;
	virtual void applyMove(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) override;
	virtual void applyJump(GMPhysicsObject* phy, const GMPhysicsMoveArgs& args) override;

public:
	GMBSPPhysicsWorld::Data& physicsData();
	void initBSPPhysicsWorld();

private:
	GMBSPMove* getMove(GMPhysicsObject* o);
	void generatePhysicsPlaneData();
	void generatePhysicsBrushSideData();
	void generatePhysicsBrushData();
	void generatePhysicsPatches();
};

inline GMBSPPhysicsObject* gmBSPPhysicsObjectCast(GMPhysicsObject* obj)
{
	return gm_cast<GMBSPPhysicsObject*>(obj);
}

inline GMBSPPhysicsObject& gmBSPPhysicsObjectCast(GMPhysicsObject& obj)
{
	return *gm_cast<GMBSPPhysicsObject*>(&obj);
}

END_NS
#endif