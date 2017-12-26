#ifndef __PHYSICSOBJECTFACTORY_H__
#define __PHYSICSOBJECTFACTORY_H__
#include <gmcommon.h>
#include "gmphysicsstructs.h"
BEGIN_NS

struct CollisionObjectFactory
{
	static GMPhysicsObject defaultCamera();
};

END_NS
#endif