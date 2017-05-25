#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include "common.h"
#include "physicsstructs.h"
BEGIN_NS

class GameWorld;
struct CollisionObject;
GM_PRIVATE_OBJECT(PhysicsWorld)
{
	GameWorld* world;
	GMfloat gravity;
};

class PhysicsWorld : public GMObject
{
	DECLARE_PRIVATE(PhysicsWorld)

public:
	PhysicsWorld(GameWorld* world);
	virtual ~PhysicsWorld();

public:
	virtual void simulate() = 0;
	virtual CollisionObject* find(GameObject* obj) = 0;
	virtual void sendCommand(CollisionObject* obj, const CommandParams& dataParam) = 0;

public:
	static CommandParams makeCommand(Command cmd, CommandVector3* list, GMuint count);
};

END_NS
#endif