#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include "common.h"
#include "physicsworldprivate.h"
BEGIN_NS

class GameObject;
class Character;
class PhysicsWorld
{
	DEFINE_PRIVATE(PhysicsWorld)

public:
	void addToPhysics(GameObject* obj);
	void simulate();
	CollisionObject* find(GameObject* o);

};

END_NS
#endif