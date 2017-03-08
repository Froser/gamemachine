#ifndef __BSPPHYSICSWORLD_H__
#define __BSPPHYSICSWORLD_H__
#include "common.h"
#include "physicsworld.h"
#include "bspphysicsworldprivate.h"
BEGIN_NS

class BSPGameWorld;
class BSPPhysicsWorld : public PhysicsWorld
{
	DEFINE_PRIVATE(BSPPhysicsWorld)

public:
	BSPPhysicsWorld(GameWorld* world);

public:
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
	BSPPatchCollide* generatePatchCollide(GMint width, GMint height, const vmath::vec3* points, OUT BSPPatchCollide** pc);

	// trace
private:
	void trace(const vmath::vec3& start, const vmath::vec3& end, const vmath::vec3& origin, const vmath::vec3& mins, const vmath::vec3& maxs, REF BSPTrace& trace);
	void traceThroughTree(BSPTraceWork& tw, GMint num, GMfloat p1f, GMfloat p2f, vmath::vec3 p1, vmath::vec3 p2);
	void traceThroughLeaf(BSPTraceWork& tw, BSPLeaf* leaf);
	void traceThroughBrush(BSPTraceWork& tw, BSP_Physics_Brush* brush);
	void traceThroughPatch(BSPTraceWork& tw, BSP_Physics_Patch* patch);
	void traceThroughPatchCollide(BSPTraceWork& tw, BSPPatchCollide* pc);
	void tracePointThroughPatchCollide(BSPTraceWork& tw, const BSPPatchCollide *pc);
	GMint checkFacetPlane(const vmath::vec4& plane, const vmath::vec3& start, const vmath::vec3& end, GMfloat *enterFrac, GMfloat *leaveFrac, GMint *hit);

	void positionTest(BSPTraceWork& tw);
	void getTouchedLeafs(REF BSPLeafList& leafList, int nodeNum);
	void storeLeafs(REF BSPLeafList& lst, GMint nodeNum);
};

END_NS
#endif