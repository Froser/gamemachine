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
	void setCamera(GameObject* obj);

private:
	void getTouchedLeafs(GMint nodeNum);
};

END_NS
#endif