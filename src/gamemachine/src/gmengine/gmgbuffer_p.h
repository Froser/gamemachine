#ifndef __GMGBUFFER_P_H__
#define __GMGBUFFER_P_H__
#include <gmcommon.h>
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMGBuffer)
{
	GM_DECLARE_PUBLIC(GMGBuffer)
	const IRenderContext* context = nullptr;
	IFramebuffers* geometryFramebuffers = nullptr;
	GMGameObject* quad = nullptr;
	GMGeometryPassingState state = GMGeometryPassingState::Done;
	GMGraphicEngine* engine = nullptr;
	void createQuad();
};

END_NS
#endif