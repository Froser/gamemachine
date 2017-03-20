#ifndef __BSPPHYSICSWORLDPRIVATE_H__
#define __BSPPHYSICSWORLDPRIVATE_H__
#include "common.h"
#include "gmphysics/physicsstructs.h"
#include <vector>
#include "gmdatacore/bsp/bsp.h"
#include "bspphysicsstructs.h"
#include "bsptrace.h"
BEGIN_NS

class BSPGameWorld;
struct BSPPhysicsWorldPrivate
{

	BSPPhysicsWorldPrivate::BSPPhysicsWorldPrivate()
	{
	}

	~BSPPhysicsWorldPrivate();

	BSPGameWorld* world;
	CollisionObject camera;

	std::vector<BSPTracePlane> planes;
	std::vector<BSP_Physics_Brush> brushes;
	std::vector<BSP_Physics_BrushSide> brushsides;
	AUTORELEASE std::vector<BSP_Physics_Patch*> patches;

	BSPTrace trace;
};

typedef BSPPhysicsWorldPrivate BSPPhysicsWorldData;
END_NS
#endif