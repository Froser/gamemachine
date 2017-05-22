#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include "common.h"
#include "physicsworldprivate.h"
BEGIN_NS

class GameWorld;
class PhysicsWorld
{
	DEFINE_PRIVATE(PhysicsWorld)

public:
	PhysicsWorld(GameWorld* world);
	virtual ~PhysicsWorld();

public:
	virtual void simulate() = 0;
	virtual CollisionObject* find(GameObject* obj) = 0;
	virtual void sendCommand(CollisionObject* obj, const CommandParams& dataParam) = 0;

public:
	static CommandParams makeCommand(Command cmd, linear_math::Vector3* list, GMuint count);
};

END_NS
#endif