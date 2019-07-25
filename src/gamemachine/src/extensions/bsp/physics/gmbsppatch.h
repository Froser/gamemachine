#ifndef __BSPPATCH_H__
#define __BSPPATCH_H__
#include <gmcommon.h>
#include <linearmath.h>

BEGIN_NS

struct GMBSP_Physics_Patch;

GM_PRIVATE_CLASS(GMBSPPatch);
class GMBSPPatch
{
	GM_DECLARE_PRIVATE(GMBSPPatch)
	GM_DISABLE_COPY_ASSIGN(GMBSPPatch)

public:
	GMBSPPatch();
	~GMBSPPatch();

	void alloc(GMint32 num);
	GMBSP_Physics_Patch* patches(GMint32 at);
	void generatePatchCollide(GMint32 index, GMint32 width, GMint32 height, const GMVec3* points, AUTORELEASE GMBSP_Physics_Patch* patch);
};

END_NS
#endif