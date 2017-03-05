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
	void trace(const vmath::vec3& start, const vmath::vec3& end, const vmath::vec3& origin);
	void positionTest(BSPTraceWork& tw);
	void getTouchedLeafs(REF BSPLeafList& leafList, int nodeNum);
	void storeLeafs(REF BSPLeafList& lst, GMint nodeNum);
};

END_NS
#endif