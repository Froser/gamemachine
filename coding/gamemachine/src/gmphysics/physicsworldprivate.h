#ifndef __PHYSICSWORLD_PRIVATE_H__
#define __PHYSICSWORLD_PRIVATE_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "physicsstructs.h"
BEGIN_NS

class GameWorld;
struct PhysicsWorldPrivate
{
	PhysicsWorldPrivate();

	GameWorld* world;
	GMfloat gravity;
};

END_NS
#endif