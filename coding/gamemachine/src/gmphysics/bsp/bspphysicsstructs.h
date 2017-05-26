#ifndef __BSPPHYSICSSTRUCTS_H__
#define __BSPPHYSICSSTRUCTS_H__
#include "common.h"
#include "gmdatacore/bsp/bsp.h"
#include "foundation/linearmath.h"
#include "foundation/vector.h"
BEGIN_NS

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )
enum PlaneType
{
	PLANE_X = 0,
	PLANE_Y,
	PLANE_Z,
	PLANE_NON_AXIAL,
};

GM_ALIGNED_STRUCT(BSPLeafList)
{
	linear_math::Vector3 bounds[2];
	AlignedVector<GMint> list;
	GMint lastLeaf;
};

struct BSPTracePlane;
GM_ALIGNED_STRUCT(BSP_Physics_BrushSide)
{
	BSPBrushSide* side;
	BSPTracePlane* plane;
	GMint surfaceFlags;
};

GM_ALIGNED_STRUCT(BSP_Physics_Brush)
{
	BSP_Physics_Brush()
		: checkcount(0)
	{
	}

	BSPBrush* brush;
	GMint contents;
	linear_math::Vector3 bounds[2];
	BSP_Physics_BrushSide *sides;
	GMint checkcount;
};

// Begin patches definitions
GM_ALIGNED_STRUCT(BSPPatchPlane)
{
	linear_math::Vector4 plane;
	GMint signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
};

GM_ALIGNED_STRUCT(BSPFacet)
{
	GMint surfacePlane;
	GMint numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	GMint borderPlanes[4 + 6 + 16];
	GMint borderInward[4 + 6 + 16];
	bool borderNoAdjust[4 + 6 + 16];
};

GM_ALIGNED_STRUCT(BSPPatchCollide)
{
	linear_math::Vector3 bounds[2];
	AlignedVector<BSPPatchPlane> planes;
	AlignedVector<BSPFacet> facets;
};
// End patches definitions

GM_ALIGNED_STRUCT(BSP_Physics_Patch)
{
	BSPSurface* surface;
	GMint checkcount;
	BSPShader* shader;
	BSPPatchCollide *pc;

	BSP_Physics_Patch()
		: checkcount(0)
		, pc(nullptr)
	{
	}

	~BSP_Physics_Patch()
	{
		if (pc)
			delete pc;
	}
};

END_NS
#endif