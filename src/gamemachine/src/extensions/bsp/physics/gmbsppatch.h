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
	GM_DECLARE_PRIVATE(GMBSPPatch)

public:
	void alloc(GMint32 num);
	GMBSP_Physics_Patch* patches(GMint32 at);
	void generatePatchCollide(GMint32 index, GMint32 width, GMint32 height, const GMVec3* points, AUTORELEASE GMBSP_Physics_Patch* patch);
};

END_NS
#endif