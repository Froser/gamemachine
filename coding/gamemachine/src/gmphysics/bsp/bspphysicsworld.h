#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include "common.h"
#include "gmphysics/physicsworld.h"
#include "bspphysicsworldprivate.h"
BEGIN_NS

class BSPGameWorld;
class BSPPhysicsWorld : public PhysicsWorld
{
	DEFINE_PRIVATE(BSPPhysicsWorld)

public:
	BSPPhysicsWorld(GameWorld* world);

public:
	BSPPhysicsWorldData& physicsData();
	virtual void simulate() override;
	virtual CollisionObject* find(GameObject* obj) override;

public:
	void initBSPPhysicsWorld();
	void setCamera(GameObject* obj);

private:
	void generatePhysicsPlaneData();
	void generatePhysicsBrushSideData();
	void generatePhysicsBrushData();
	void generatePhysicsPatches();
};

END_NS
#endif