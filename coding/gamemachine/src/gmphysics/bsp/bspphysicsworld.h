#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include "common.h"
#include "gmphysics/physicsworld.h"
#include "bspphysicsworldprivate.h"
BEGIN_NS

class BSPGameWorld;
class BSPMove;
class BSPPhysicsWorld : public PhysicsWorld
{
	DEFINE_PRIVATE(BSPPhysicsWorld)

public:
	BSPPhysicsWorld(GameWorld* world);
	~BSPPhysicsWorld();

public:
	virtual void simulate() override;
	virtual CollisionObject* find(GameObject* obj) override;
	virtual void sendCommand(CollisionObject* obj, const CommandParams& dataParam) override;

public:
	BSPPhysicsWorldData& physicsData();
	void initBSPPhysicsWorld();
	void setCamera(GameObject* obj);

private:
	BSPMove* getMove(CollisionObject* o);
	void generatePhysicsPlaneData();
	void generatePhysicsBrushSideData();
	void generatePhysicsBrushData();
	void generatePhysicsPatches();
};

END_NS
#endif