#ifndef __BSPPHYSICSWORLD_P_H__
#define __BSPPHYSICSWORLD_P_H__
#include <gmcommon.h>
#include "../../../gmphysics/gmphysicsworld_p.h"
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED_FROM(GMBSPPhysicsWorld, GMPhysicsWorld)
{
	GMBSPGameWorld* world;

	AlignedVector<BSPTracePlane> planes;
	AlignedVector<GMBSP_Physics_Brush> brushes;
	AlignedVector<GMBSP_Physics_BrushSide> brushsides;

	GMBSPTrace trace;
	GMBSPPatch patch;

	Map<GMPhysicsObject*, GMBSPMove*> objectMoves;
	Map<GMGameObject*, GMPhysicsObject*> collisionObjects;
};

END_NS
#endif