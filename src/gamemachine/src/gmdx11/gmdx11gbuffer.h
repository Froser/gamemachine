#ifndef __GMDX11GBUFFER_H__
#define __GMDX11GBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

class GMDx11GraphicEngine;
class GMGameObject;
GM_PRIVATE_OBJECT(GMDx11GBuffer)
{
	GMDx11GraphicEngine* engine = nullptr;
	IFramebuffers* geometryFramebuffer = nullptr;
	IFramebuffers* materialFramebuffer = nullptr;
	GMGameObject* quad = nullptr;
	GMModel* quadModel = nullptr;
};

class GMDx11GBuffer : public GMObject
{
	DECLARE_PRIVATE(GMDx11GBuffer)

public:
	GMDx11GBuffer(GMDx11GraphicEngine* engine);
	~GMDx11GBuffer();

public:
	void init();
	void geometryPass(GMGameObject *objects[], GMuint count);
	void lightPass();
	IFramebuffers* getGeometryFramebuffers();
	IFramebuffers* getMaterialFramebuffers();
};

END_NS
#endif