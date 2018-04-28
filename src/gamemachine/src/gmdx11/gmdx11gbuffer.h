#ifndef __GMDX11GBUFFER_H__
#define __GMDX11GBUFFER_H__
#include <gmcommon.h>
#include <gmgbuffer.h>
BEGIN_NS

class GMDx11GraphicEngine;
class GMGameObject;
class GMDx11GBuffer : public GMGBuffer
{
public:
	GMDx11GBuffer(GMGraphicEngine* engine);

public:
	virtual void geometryPass(const List<GMGameObject*>& objects) override;
	virtual void lightPass() override;

	void useGeometryTextures(ID3DX11Effect* effect);

protected:
	virtual IFramebuffers* createGeometryFramebuffers() override;

private:
	bool isMultisamping();
};

END_NS
#endif