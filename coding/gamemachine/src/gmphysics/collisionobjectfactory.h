#ifndef __PHYSICSOBJECTFACTORY_H__
#define __PHYSICSOBJECTFACTORY_H__
#include "common.h"
#include "gmphysicsstructs.h"
BEGIN_NS

struct CollisionObjectFactory
{
	static GMCollisionObject defaultCamera();
};

END_NS
#endif