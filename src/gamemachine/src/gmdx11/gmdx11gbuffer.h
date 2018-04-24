#ifndef __GMDX11GBUFFER_H__
#define __GMDX11GBUFFER_H__
#include <gmcommon.h>
#include <gmgbuffer.h>
BEGIN_NS

class GMDx11GraphicEngine;
class GMGameObject;
class GMDx11Texture;
GM_PRIVATE_OBJECT(GMDx11GBuffer)
{
	GMDx11GraphicEngine* engine = nullptr;
};

class GMDx11GBuffer : public GMGBuffer
{
	DECLARE_PRIVATE(GMDx11GBuffer)

public:
	GMDx11GBuffer(GMDx11GraphicEngine* engine);

public:
	virtual void geometryPass(GMGameObject *objects[], GMuint count) override;
	virtual void lightPass() override;

	void useGeometryTextures(ID3DX11Effect* effect);
	void useMaterialTextures(ID3DX11Effect* effect);

private:
	void setSampler(ID3DX11Effect* effect, GMDx11Texture* texture);

protected:
	virtual IFramebuffers* createGeometryFramebuffers() override;
	virtual IFramebuffers* createMaterialFramebuffers() override;
};

END_NS
#endif