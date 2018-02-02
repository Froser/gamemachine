#ifndef __BSPPATCH_H__
#define __BSPPATCH_H__
#include <gmcommon.h>
#include <linearmath.h>

BEGIN_NS

struct GMBSP_Physics_Patch;
GM_PRIVATE_OBJECT(GMBSPPatch)
{
	GM_PRIVATE_DESTRUCT(GMBSPPatch);
	AUTORELEASE AlignedVector<GMBSP_Physics_Patch*> patches;
};

struct GMBSPPatchCollide;
class GMBSPPatch : public GMObject
{
	DECLARE_PRIVATE(GMBSPPatch)

public:
	void alloc(GMint num);
	GMBSP_Physics_Patch* patches(GMint at);
	void generatePatchCollide(GMint index, GMint width, GMint height, const GMVec3* points, AUTORELEASE GMBSP_Physics_Patch* patch);
};

END_NS
#endif