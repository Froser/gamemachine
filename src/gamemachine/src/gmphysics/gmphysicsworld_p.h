#ifndef __PHYSICSWORLD_P_H__
#define __PHYSICSWORLD_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMPhysicsWorld)
{
	GMGameWorld* world;
	GMfloat gravity;
	bool enabled = true;
};

END_NS
#endif