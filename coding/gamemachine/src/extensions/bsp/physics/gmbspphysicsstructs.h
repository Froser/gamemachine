#ifndef __BSPPHYSICSSTRUCTS_H__
#define __BSPPHYSICSSTRUCTS_H__
#include <gmcommon.h>
#include <linearmath.h>
#include <gmbsp.h>
BEGIN_NS

#define PlaneTypeForNormal(x) (x[0] == 1.0 ? PLANE_X : (x[1] == 1.0 ? PLANE_Y : (x[2] == 1.0 ? PLANE_Z : PLANE_NON_AXIAL) ) )
enum PlaneType
{
	PLANE_X = 0,
	PLANE_Y,
	PLANE_Z,
	PLANE_NON_AXIAL,
};

GM_ALIGNED_STRUCT(GMBSPLeafList)
{
	linear_math::Vector3 bounds[2];
	AlignedVector<GMint> list;
	GMint lastLeaf;
};

struct BSPTracePlane;
GM_ALIGNED_STRUCT(GMBSP_Physics_BrushSide)
{
	GMBSPBrushSide* side;
	BSPTracePlane* plane;
	GMint surfaceFlags;
};

GM_ALIGNED_STRUCT(GMBSP_Physics_Brush)
{
	GMBSP_Physics_Brush()
		: checkcount(0)
	{
	}

	GMBSPBrush* brush;
	GMint contents;
	linear_math::Vector3 bounds[2];
	GMBSP_Physics_BrushSide *sides;
	GMint checkcount;
};

// Begin patches definitions
GM_ALIGNED_STRUCT(GMBSPPatchPlane)
{
	linear_math::Vector4 plane;
	GMint signbits;		// signx + (signy<<1) + (signz<<2), used as lookup during collision
};

GM_ALIGNED_STRUCT(GMBSPFacet)
{
	GMint surfacePlane;
	GMint numBorders;		// 3 or four + 6 axial bevels + 4 or 3 * 4 edge bevels
	GMint borderPlanes[4 + 6 + 16];
	GMint borderInward[4 + 6 + 16];
	bool borderNoAdjust[4 + 6 + 16];
};

GM_ALIGNED_STRUCT(GMBSPPatchCollide)
{
	linear_math::Vector3 bounds[2];
	AlignedVector<GMBSPPatchPlane> planes;
	AlignedVector<GMBSPFacet> facets;
};
// End patches definitions

GM_ALIGNED_STRUCT(GMBSP_Physics_Patch)
{
	GMBSPSurface* surface = nullptr;
	GMBSPShader* shader = nullptr;
	GMBSPPatchCollide *pc = nullptr;
	GMint checkcount = 0;

	GMBSP_Physics_Patch()
		: checkcount(0)
		, pc(nullptr)
	{
	}

	~GMBSP_Physics_Patch()
	{
		if (pc)
			delete pc;
	}
};

END_NS
#endif