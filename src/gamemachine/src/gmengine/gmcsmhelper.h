#ifndef __GMCSMHELPER_H__
#define __GMCSMHELPER_H__
#include <gmcommon.h>
BEGIN_NS

struct GMShadowSourceDesc;
struct GMCSMHelper
{
	static void setOrthoCamera(ICSMFramebuffers* csm, const GMCamera& viewerCamera, const GMShadowSourceDesc& shadowSourceDesc, GMCamera& shadowCamera);
};

END_NS
#endif