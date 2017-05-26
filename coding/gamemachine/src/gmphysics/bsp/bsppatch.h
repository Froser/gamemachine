#ifndef __BSPPATCH_H__
#define __BSPPATCH_H__
#include "common.h"
#include "foundation/linearmath.h"
#include "foundation/vector.h"

BEGIN_NS

struct BSP_Physics_Patch;
GM_PRIVATE_OBJECT(BSPPatch)
{
	~GM_PRIVATE_CONSTRUCT(BSPPatch);
	AUTORELEASE AlignedVector<BSP_Physics_Patch*> patches;
};

struct BSPPatchCollide;
GM_ALIGNED_16(class) BSPPatch : public GMObject
{
	DECLARE_PRIVATE(BSPPatch)

public:
	void alloc(GMint num);
	BSP_Physics_Patch* patches(GMint at);
	void generatePatchCollide(GMint index, GMint width, GMint height, const linear_math::Vector3* points, AUTORELEASE BSP_Physics_Patch* patch);
};

END_NS
#endif