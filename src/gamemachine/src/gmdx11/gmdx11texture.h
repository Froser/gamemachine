#ifndef __GMDX11TEXTURE_H__
#define __GMDX11TEXTURE_H__
#include <gmcommon.h>
#include <gmdxincludes.h>
#include <gmcom.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11Texture)
{
	GMImage* image = nullptr;
	GMComPtr<ID3D11Device> device;
	GMComPtr<ID3D11ShaderResourceView> shaderResourceView;
};

class GMDx11Texture : public ITexture
{
	DECLARE_PRIVATE(GMDx11Texture)

public:
	GMDx11Texture(GMImage* image);

public:
	virtual void drawTexture(GMTextureFrames* frames) override;

private:
	void init();
};

END_NS
#endif