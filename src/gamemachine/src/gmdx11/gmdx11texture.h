#ifndef __GMDX11TEXTURE_H__
#define __GMDX11TEXTURE_H__
#include <gmcommon.h>
#include <gmdxincludes.h>
#include <gmcom.h>
BEGIN_NS

struct GMDx11TextureSamplerVariable
{
	ID3DX11EffectShaderResourceVariable* shaderResource;
	ID3DX11EffectSamplerVariable* sampler;
};

GM_PRIVATE_CLASS(GMDx11Texture);
class GMDx11Texture : public ITexture
{
	GM_DECLARE_PRIVATE(GMDx11Texture)
	GM_DISABLE_COPY_ASSIGN(GMDx11Texture)

public:
	GMDx11Texture(const IRenderContext* context, GMImage* image);
	~GMDx11Texture();

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureType) override;
	virtual const IRenderContext* getContext();

public:
	ID3D11ShaderResourceView* getResourceView();
};

class GMDx11WhiteTexture : public GMDx11Texture
{
public:
	GMDx11WhiteTexture(const IRenderContext* context);

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler*) override;
};

class GMDx11EmptyTexture : public GMDx11WhiteTexture
{
public:
	using GMDx11WhiteTexture::GMDx11WhiteTexture;
	virtual void init() override;
};

END_NS
#endif