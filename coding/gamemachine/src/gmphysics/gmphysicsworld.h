#ifndef __PHYSICSWORLD_H__
#define __PHYSICSWORLD_H__
#include "common.h"
#include "gmphysicsstructs.h"
BEGIN_NS

class GMGameWorld;
struct GMCollisionObject;
GM_PRIVATE_OBJECT(GMPhysicsWorld)
{
	GMGameWorld* world;
	GMfloat gravity;
};

class GMPhysicsWorld : public GMObject
{
	DECLARE_PRIVATE(GMPhysicsWorld)

public:
	GMPhysicsWorld(GMGameWorld* world);
	virtual ~GMPhysicsWorld();

public:
	virtual void simulate() = 0;
	virtual GMCollisionObject* find(GMGameObject* obj) = 0;
	virtual void sendCommand(GMCollisionObject* obj, const CommandParams& dataParam) = 0;

public:
	static CommandParams makeCommand(GMCommand cmd, GMCommandVector3* list, GMuint count);
};

END_NS
#endif