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

GM_PRIVATE_OBJECT_UNALIGNED(GMDx11Texture)
{
	const IRenderContext* context = nullptr;
	GMImage* image = nullptr;
	GMComPtr<ID3D11Device> device;
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMComPtr<ID3D11Resource> resource;
	GMComPtr<ID3D11ShaderResourceView> shaderResourceView;
	GMComPtr<ID3D11SamplerState> samplerState;
	GMComPtr<ID3DX11Effect> effect;
	HashMap<const char*, GMDx11TextureSamplerVariable> variables;
};

class GMDx11Texture : public ITexture
{
	GM_DECLARE_PRIVATE_NGO(GMDx11Texture)

public:
	GMDx11Texture(const IRenderContext* context, GMImage* image);
	~GMDx11Texture();

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureType) override;

	virtual const IRenderContext* getContext();

public:
	ID3D11ShaderResourceView* getResourceView()
	{
		D(d);
		GM_ASSERT(d->shaderResourceView);
		return d->shaderResourceView;
	}
};

class GMDx11WhiteTexture : public GMDx11Texture
{
public:
	GMDx11WhiteTexture(const IRenderContext* context)
		: GMDx11Texture(context, nullptr)
	{
	}

	virtual void init() override;
	virtual void bindSampler(GMTextureSampler*) override;
};


END_NS
#endif