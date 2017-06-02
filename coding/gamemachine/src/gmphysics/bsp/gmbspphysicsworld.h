#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include "common.h"
#include "gmphysics/gmphysicsworld.h"
#include "gmbsptrace.h"
#include "gmbsppatch.h"
#include "gmbspphysicsstructs.h"
BEGIN_NS

class GMBSPGameWorld;
class GMBSPMove;

GM_PRIVATE_OBJECT_FROM(GMBSPPhysicsWorld, GMPhysicsWorld)
{
	GMBSPGameWorld* world;
	GMCollisionObject camera;

	AlignedVector<BSPTracePlane> planes;
	AlignedVector<GMBSP_Physics_Brush> brushes;
	AlignedVector<GMBSP_Physics_BrushSide> brushsides;

	GMBSPTrace trace;
	GMBSPPatch patch;
	std::map<GMCollisionObject*, GMBSPMove*> objectMoves;
};

class GMBSPPhysicsWorld : public GMPhysicsWorld
{
	DECLARE_PRIVATE(GMBSPPhysicsWorld)

public:
	GMBSPPhysicsWorld(GMGameWorld* world);
	~GMBSPPhysicsWorld();

public:
	virtual void simulate() override;
	virtual GMCollisionObject* find(GMGameObject* obj) override;
	virtual void sendCommand(GMCollisionObject* obj, const CommandParams& dataParam) override;

public:
	GMBSPPhysicsWorld::Data& physicsData();
	void initBSPPhysicsWorld();
	void setCamera(GMGameObject* obj);

private:
	GMBSPMove* getMove(GMCollisionObject* o);
	void generatePhysicsPlaneData();
	void generatePhysicsBrushSideData();
	void generatePhysicsBrushData();
	void generatePhysicsPatches();
};

END_NS
#endif