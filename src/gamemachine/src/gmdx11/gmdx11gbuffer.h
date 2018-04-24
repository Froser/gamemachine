#ifndef __GMDX11GBUFFER_H__
#define __GMDX11GBUFFER_H__
#include <gmcommon.h>
BEGIN_NS

class GMDx11GraphicEngine;
class GMGameObject;
class GMDx11Texture;
GM_PRIVATE_OBJECT(GMDx11GBuffer)
{
	GMDx11GraphicEngine* engine = nullptr;
	IFramebuffers* geometryFramebuffers = nullptr;
	IFramebuffers* materialFramebuffers = nullptr;
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
	void useGeometryTextures(ID3DX11Effect* effect);
	void useMaterialTextures(ID3DX11Effect* effect);
	IFramebuffers* getGeometryFramebuffers();
	IFramebuffers* getMaterialFramebuffers();

private:
	void setSampler(ID3DX11Effect* effect, GMDx11Texture* texture);
};

END_NS
#endif