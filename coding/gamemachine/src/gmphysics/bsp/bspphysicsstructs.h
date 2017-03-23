#ifndef __BSPPHYSICSSTRUCTS_H__
#define __BSPPHYSICSSTRUCTS_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "utilities/vmath.h"
BEGIN_NS

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )
enum PlaneType
{
	PLANE_X = 0,
	PLANE_Y,
	PLANE_Z,
	PLANE_NON_AXIAL,
};

struct BSPLeafList
{
	std::vector<GMint> list;
	vmath::vec3 bounds[2];
	GMint lastLeaf;
};

struct BSPPatchPlane
{
	vmath::vec4 plane;
	GMint signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
};

struct BSPFacet
{
	GMint surfacePlane;
	GMint numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	GMint borderPlanes[4 + 6 + 16];
	GMint borderInward[4 + 6 + 16];
	bool borderNoAdjust[4 + 6 + 16];
};

struct BSPPatchCollide
{
	~BSPPatchCollide()
	{
		if (numPlanes > 0)
			delete[] planes;
		if (numFacets > 0)
			delete[] facets;
	}

	vmath::vec3 bounds[2];
	int numPlanes; // surface planes plus edge planes
	BSPPatchPlane *planes; //need release
	int numFacets;
	BSPFacet* facets; //need release
};

struct BSPTracePlane;
struct BSP_Physics_BrushSide
{
	BSPBrushSide* side;
	BSPTracePlane* plane;
	GMint surfaceFlags;
};

struct BSP_Physics_Brush
{
	BSP_Physics_Brush()
		: checkcount(0)
	{
	}

	BSPBrush* brush;
	GMint contents;
	vmath::vec3 bounds[2];
	BSP_Physics_BrushSide *sides;
	GMint checkcount;
};

struct BSP_Physics_Patch
{
	BSPSurface* surface;
	GMint checkcount;
	BSPShader* shader;
	BSPPatchCollide *pc;

	BSP_Physics_Patch()
		: checkcount(0)
	{
	}

	~BSP_Physics_Patch()
	{
		delete pc;
	}
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


END_NS
#endif