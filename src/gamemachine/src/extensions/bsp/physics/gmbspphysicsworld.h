#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include <gmcommon.h>
#include <gmphysicsworld.h>
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

	Map<GMPhysicsObject*, GMBSPMove*> objectMoves;
	Map<GMGameObject*, GMPhysicsObject*> collisionObjects;
};

class GMBSPPhysicsWorld : public GMPhysicsWorld
{
	DECLARE_PRIVATE_AND_BASE(GMBSPPhysicsWorld, GMPhysicsWorld)

public:
	GMBSPPhysicsWorld(GMGameWorld* world);
	~GMBSPPhysicsWorld();

public:
	virtual void simulate(GMGameObject* obj) override;
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
	return gm_static_cast<GMBSPPhysicsObject*>(obj);
}

inline GMBSPPhysicsObject& gmBSPPhysicsObjectCast(GMPhysicsObject& obj)
{
	return *gm_static_cast<GMBSPPhysicsObject*>(&obj);
}

END_NS
#endif