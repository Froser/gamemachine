#ifndef __BSPPHYSICSWORLDPRIVATE_H__
#define __BSPPHYSICSWORLDPRIVATE_H__
#include "common.h"
#include "physicsstruct.h"
BEGIN_NS

class BSPGameWorld;
struct BSPPhysicsWorldPrivate
{
	BSPGameWorld* world;
	CollisionObject camera;
};

END_NS
#endif