#ifndef __BSPPATCH_H__
#define __BSPPATCH_H__
#include "common.h"
#include "utilities/linearmath.h"
#include "utilities/vector.h"

BEGIN_NS

struct BSP_Physics_Patch;
struct BSPPatchPrivate
{
	~BSPPatchPrivate();
	AUTORELEASE Vector<BSP_Physics_Patch*> patches;
};

struct BSPPatchCollide;
class BSPPatch
{
	DEFINE_PRIVATE(BSPPatch)

public:
	void alloc(GMint num);
	BSP_Physics_Patch* patches(GMint at);
	void generatePatchCollide(GMint index, GMint width, GMint height, const linear_math::Vector3* points, AUTORELEASE BSP_Physics_Patch* patch);
};

END_NS
#endif