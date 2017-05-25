#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include "common.h"
#include "gmphysics/physicsworld.h"
#include "bsptrace.h"
#include "bsppatch.h"
BEGIN_NS

class BSPGameWorld;
class BSPMove;

class BSPGameWorld;
class BSPMove;

GM_PRIVATE_OBJECT_FROM(BSPPhysicsWorld, PhysicsWorld)
{
	BSPGameWorld* world;
	CollisionObject camera;

	AlignedVector<BSPTracePlane> planes;
	AlignedVector<BSP_Physics_Brush> brushes;
	AlignedVector<BSP_Physics_BrushSide> brushsides;

	BSPTrace trace;
	BSPPatch patch;
	std::map<CollisionObject*, BSPMove*> objectMoves;
};

class BSPPhysicsWorld : public PhysicsWorld
{
	DECLARE_PRIVATE(BSPPhysicsWorld)

public:
	BSPPhysicsWorld(GameWorld* world);
	~BSPPhysicsWorld();

public:
	virtual void simulate() override;
	virtual CollisionObject* find(GameObject* obj) override;
	virtual void sendCommand(CollisionObject* obj, const CommandParams& dataParam) override;

public:
	BSPPhysicsWorld::Data& physicsData();
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