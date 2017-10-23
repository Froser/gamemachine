#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include <gmcommon.h>
#include <gmphysics.h>
#include <gmbspphysicsworld.h>
#include "gmbsptrace.h"
#include "gmbsppatch.h"
BEGIN_NS

class GMBSPGameWorld;
class GMBSPMove;

GM_PRIVATE_OBJECT_FROM(GMBSPPhysicsWorld, GMPhysicsWorld)
{
	GMBSPGameWorld* world;

	AlignedVector<BSPTracePlane> planes;
	AlignedVector<GMBSP_Physics_Brush> brushes;
	AlignedVector<GMBSP_Physics_BrushSide> brushsides;

	GMBSPTrace trace;
	GMBSPPatch patch;

	Map<GMCollisionObject*, GMBSPMove*> objectMoves;
	Map<GMGameObject*, GMCollisionObject*> collisionObjects;
};

class GMBSPPhysicsWorld : public GMPhysicsWorld
{
	DECLARE_PRIVATE(GMBSPPhysicsWorld)

public:
	GMBSPPhysicsWorld(GMGameWorld* world);
	~GMBSPPhysicsWorld();

public:
	virtual void simulate(GMGameObject* obj) override;
	virtual GMCollisionObject* find(GMGameObject* obj) override;
	virtual void sendCommand(GMCollisionObject* obj, const CommandParams& dataParam) override;

public:
	GMBSPPhysicsWorld::Data& physicsData();
	void initBSPPhysicsWorld();

private:
	GMBSPMove* getMove(GMCollisionObject* o);
	void generatePhysicsPlaneData();
	void generatePhysicsBrushSideData();
	void generatePhysicsBrushData();
	void generatePhysicsPatches();
};

END_NS
#endif