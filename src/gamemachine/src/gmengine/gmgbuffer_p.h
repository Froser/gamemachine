#ifndef __GMGBUFFER_P_H__
#define __GMGBUFFER_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGBuffer)
{
	const IRenderContext* context = nullptr;
	IFramebuffers* geometryFramebuffers = nullptr;
	GMGameObject* quad = nullptr;
	GMGeometryPassingState state = GMGeometryPassingState::Done;
	GMGraphicEngine* engine = nullptr;
};

END_NS
#endif