#ifndef __BSPPHYSICSWORLDPRIVATE_H__
#define __BSPPHYSICSWORLDPRIVATE_H__
#include "common.h"
#include "physicsstruct.h"
#include <vector>
#include "gmdatacore/bsp/bsp.h"
BEGIN_NS

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )
enum PlaneType
{
	PLANE_X = 0,
	PLANE_Y,
	PLANE_Z,
	PLANE_NON_AXIAL,
};

struct BSP_Physics_Plane
{
	BSPPlane* plane;
	PlaneType planeType;
	GMint signbits;

	GMint classifyBox(vmath::vec3& emins, vmath::vec3& emaxs);
};

struct BSPLeafList
{
	std::vector<GMint> list;
	vmath::vec3 bounds[2];
	GMint lastLeaf;
};

struct BSPPatchPlane {
	vmath::vec4 plane;
	int signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
};

typedef struct {
	GMint surfacePlane;
	GMint numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	GMint borderPlanes[4 + 6 + 16];
	GMint borderInward[4 + 6 + 16];
	bool borderNoAdjust[4 + 6 + 16];
} BSPFacet;

struct BSPPatchCollide
{
	vmath::vec3 bounds[2];
	int numPlanes; // surface planes plus edge planes
	BSPPatchPlane	*planes; // TODO need release
	int numFacets;
	BSPFacet* facets; //TODO need release
};

struct BSPTrace
{
	bool allsolid; // if true, plane is not valid
	bool startsolid; // if true, the initial point was in a solid area
	float fraction; // time completed, 1.0 = didn't hit anything
	vmath::vec3 endpos; // final position
	BSP_Physics_Plane plane; // surface normal at impact, transformed to world space
	int surfaceFlags; // surface hit
	int contents; // contents on other side of surface hit
	int entityNum; // entity the contacted sirface is a part of
};

struct BSPSphere
{
	bool use;
	GMfloat radius;
	GMfloat halfheight;
	vmath::vec3 offset;
};

struct BSP_Physics_BrushSide
{
	BSPBrushSide* side;
	BSP_Physics_Plane* plane;
	GMint surfaceFlags;
};

struct BSP_Physics_Brush
{
	BSPBrush* brush;
	GMint contents;
	vmath::vec3 bounds[2];
	BSP_Physics_BrushSide *sides;
	GMint checkcount;
};

struct BSP_Physics_Patch
{
	int checkcount;
	int surfaceFlags;
	int contents;
	BSPPatchCollide *pc;

	~BSP_Physics_Patch()
	{
		delete pc;
	}
};

struct BSPTraceWork
{
	vmath::vec3 start;
	vmath::vec3 end;
	vmath::vec3 size[2];	// size of the box being swept through the model
	vmath::vec3 offsets[8];	// [signbits][x] = either size[0][x] or size[1][x]
	GMfloat maxOffset;	// longest corner length from origin
	vmath::vec3 extents;	// greatest of abs(size[0]) and abs(size[1])
	vmath::vec3 bounds[2];	// enclosing box of start and end surrounding by size
	vmath::vec3 modelOrigin;// origin of the model tracing through
	GMint contents; // ored contents of the model tracing through
	bool isPoint; // optimized case
	BSPTrace trace; // returned from trace call
	BSPSphere sphere; // sphere for oriendted capsule collision
};

#define	MAX_GRID_SIZE	129

struct BSPGrid
{
	GMint width;
	GMint height;
	bool wrapWidth;
	bool wrapHeight;
	vmath::vec3 points[MAX_GRID_SIZE][MAX_GRID_SIZE];	// [width][height]
};

class BSPGameWorld;
struct BSPPhysicsWorldPrivate
{
	~BSPPhysicsWorldPrivate();

	BSPGameWorld* world;
	CollisionObject camera;

	std::vector<BSP_Physics_Plane> planes;
	std::vector<BSP_Physics_Brush> brushes;
	std::vector<BSP_Physics_BrushSide> brushsides;
	AUTORELEASE std::vector<BSP_Physics_Patch*> patches;

//tags:
	GMint checkcount;
};

END_NS
#endif