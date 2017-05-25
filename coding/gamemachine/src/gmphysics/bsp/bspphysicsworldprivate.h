#ifndef __BSPPHYSICSWORLDPRIVATE_H__
#define __BSPPHYSICSWORLDPRIVATE_H__
#include "common.h"
#include "gmphysics/physicsstructs.h"
#include "foundation/vector.h"
#include "gmdatacore/bsp/bsp.h"
#include "bspphysicsstructs.h"
#include "bsptrace.h"
#include "bsppatch.h"
#include "foundation/utilities/utilities.h"
#include "gmphysics/physicsworld.h"
BEGIN_NS

class BSPGameWorld;
class BSPMove;
struct BSPPhysicsWorldPrivate : public PhysicsWorldPrivate
{
	BSPGameWorld* world;
	CollisionObject camera;

	AlignedVector<BSPTracePlane> planes;
	AlignedVector<BSP_Physics_Brush> brushes;
	AlignedVector<BSP_Physics_BrushSide> brushsides;

	BSPTrace trace;
	BSPPatch patch;
	std::map<CollisionObject*, BSPMove*> objectMoves;
};

typedef BSPPhysicsWorldPrivate BSPPhysicsWorldData;
END_NS
#endif