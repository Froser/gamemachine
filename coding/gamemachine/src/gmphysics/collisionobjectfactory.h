#ifndef __PHYSICSOBJECTFACTORY_H__
#define __PHYSICSOBJECTFACTORY_H__
#include "common.h"
#include "physicsstructs.h"
BEGIN_NS

struct CollisionObjectFactory
{
	static CollisionObject defaultCamera();
};

END_NS
#endif