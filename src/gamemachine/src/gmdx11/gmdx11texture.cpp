#include "stdafx.h"
#include "gmdx11texture.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "gmdx11helper.h"

namespace
{
	GMMutex s_generateMipMutex;

	inline DXGI_FORMAT toDxgiFormat(GMImageFormat format)
	{
		switch (format)
		{
		case GMImageFormat::RGBA:
			return DXGI_FORMAT_R8G8B8A8_UNORM;
		case GMImageFormat::BGRA:
			return DXGI_FORMAT_B8G8R8A8_UNORM;
		case GMImageFormat::RED:
			return DXGI_FORMAT_R8_UNORM;
		default:
			GM_ASSERT(false);
			return DXGI_FORMAT_UNKNOWN;
		}
	}

	D3D11_TEXTURE_ADDRESS_MODE getAddressMode(GMS_Wrap wrapMode)
	{
		switch (wrapMode)
		{
		case GMS_Wrap::ClampToBorder:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		case GMS_Wrap::ClampToEdge:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case GMS_Wrap::Repeat:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case GMS_Wrap::MirroredRepeat:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		default:
			GM_ASSERT(false);
			return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}

#define BEGIN_TEX_SAMPLER() switch (type) {
#define TEX_SAMPLER(type, texName, samName) case type: textureName = #texName; samplerName = #samName; break
#define END_TEX_SAMPLER() }
	void getTextureAndSamplerName(GMTextureType type, REF const char*& textureName, REF const char*& samplerName)
	{
		BEGIN_TEX_SAMPLER()
			TEX_SAMPLER(GMTextureType::Ambient, GM_AmbientTexture, GM_AmbientSampler);
			TEX_SAMPLER(GMTextureType::Diffuse, GM_DiffuseTexture, GM_DiffuseSampler);
			TEX_SAMPLER(GMTextureType::Specular, GM_SpecularTexture, GM_SpecularSampler);
			TEX_SAMPLER(GMTextureType::NormalMap, GM_NormalMapTexture, GM_NormalMapSampler);
			TEX_SAMPLER(GMTextureType::Lightmap, GM_LightmapTexture, GM_LightmapSampler);
			TEX_SAMPLER(GMTextureType::Albedo, GM_AlbedoTexture, GM_AlbedoSampler);
			TEX_SAMPLER(GMTextureType::MetallicRoughnessAO, GM_MetallicRoughnessAOTexture, GM_MetallicRoughnessAOSampler);
			TEX_SAMPLER(GMTextureType::CubeMap, GM_CubeMapTexture, GM_CubeMapSampler);
		END_TEX_SAMPLER()
	}
}

GMDx11Texture::GMDx11Texture(const IRenderContext* context, GMImage* image)
{
	D(d);
	d->context = context;
	d->image = image;
	d->context->getEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&d->device);
	GM_ASSERT(d->device);

	d->context->getEngine()->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
	GM_ASSERT(d->deviceContext);
}

GMDx11Texture::~GMDx11Texture()
{
}

void GMDx11Texture::bindSampler(GMTextureSampler* sampler)
{
	D(d);
	if (!d->samplerState)
	{
		// 创建采样器
		D3D11_SAMPLER_DESC desc = GMDx11Helper::getDefaultSamplerDesc();
		if (sampler)
		{
			desc.Filter = GMDx11Helper::getTextureFilter(sampler->getMinFilter(), sampler->getMagFilter());
			desc.AddressU = getAddressMode(sampler->getWrapS());
			desc.AddressV = getAddressMode(sampler->getWrapT());
		}
		GM_DX_HR(d->device->CreateSamplerState(&desc, &d->samplerState));
	}
}

void GMDx11Texture::useTexture(GMint32 textureType)
{
	D(d);
	if (!d->effect)
	{
		bool b = d->context->getEngine()->getShaderProgram()->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&d->effect);
		GM_ASSERT(b && d->effect);
	}

#if GM_DEBUG
	// 必须确保effect没有发生变化（每一个texture对应一个effect），这样获取的Effect变量才准确
	GMComPtr<ID3DX11Effect> effect;
	d->context->getEngine()->getShaderProgram()->getInterface(GameMachineInterfaceID::D3D11Effect, (void**)&effect);
	GM_ASSERT(d->effect == effect);
#endif

	GMTextureType tt = (GMTextureType)textureType;
	GMsize_t effectId = static_cast<GMsize_t>(tt);

	if (d->shaderResourceVariables.size() <= effectId)
		d->shaderResourceVariables.resize(effectId + 1);
	if (d->samplerVariables.size() <= effectId)
		d->samplerVariables.resize(effectId + 1);
	
	const char* textureName = nullptr;
	const char* samplerName = nullptr;
	ID3DX11EffectShaderResourceVariable* shaderResourceVariable = d->shaderResourceVariables[effectId];
	ID3DX11EffectSamplerVariable* samplerVariable = d->samplerVariables[effectId];
	if (!shaderResourceVariable)
	{
		if (!textureName && !samplerName)
			getTextureAndSamplerName(tt, textureName, samplerName);
		d->shaderResourceVariables[effectId] = shaderResourceVariable = d->effect->GetVariableByName(textureName)->AsShaderResource();
	}
	if (!samplerVariable)
	{
		if (!textureName && !samplerName)
			getTextureAndSamplerName(tt, textureName, samplerName);
		d->samplerVariables[effectId] = samplerVariable = d->effect->GetVariableByName(samplerName)->AsSampler();
	}

	GM_DX_TRY(shaderResourceVariable, shaderResourceVariable->SetResource(d->shaderResourceView));
	GM_DX_TRY(samplerVariable, samplerVariable->SetSampler(0, d->samplerState));
}

const IRenderContext* GMDx11Texture::getContext()
{
	D(d);
	return d->context;
}

void GMDx11Texture::init()
{
	D(d);
	auto& imageData = d->image->getData();
	D3D11_TEXTURE2D_DESC texDesc = { 0 };

	D3D11_SUBRESOURCE_DATA* resourceData = new D3D11_SUBRESOURCE_DATA[imageData.mipLevels * imageData.slices];

	if (imageData.target == GMImageTarget::Texture2D)
	{
		GMComPtr<ID3D11Texture2D> texture;

		texDesc.Width = d->image->getWidth();
		texDesc.Height = d->image->getHeight();
		texDesc.MipLevels = imageData.generateMipmap ? -1 : imageData.mipLevels;
		texDesc.ArraySize = imageData.slices;
		texDesc.Format = toDxgiFormat(imageData.format);
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.CPUAccessFlags = 0;

		if (imageData.generateMipmap)
		{
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
			texDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		}
		else
		{
			texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
			texDesc.MiscFlags = 0;
		}

		GMint32 index = 0;
		GM_ASSERT(texDesc.ArraySize == 1);
		for (GMint32 j = 0; j < imageData.mipLevels; ++j, ++index)
		{
			GMuint32 pitch = d->image->getWidth(j);
			resourceData[index].pSysMem = imageData.mip[j].data;
			resourceData[index].SysMemPitch = pitch * d->image->getData().channels;
		}

		GM_DX_HR(d->device->CreateTexture2D(&texDesc, resourceData, &texture));
		d->resource = texture;

		GM_ASSERT(d->resource);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = texDesc.Format;
		srvDesc.ViewDimension = D3D_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = imageData.generateMipmap ? -1 : imageData.mipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		GM_DX_HR(d->device->CreateShaderResourceView(
			d->resource,
			&srvDesc,
			&d->shaderResourceView
		));

		if (imageData.generateMipmap)
		{
			if (d->context->getEngine()->isCurrentMainThread())
			{
				d->deviceContext->GenerateMips(d->shaderResourceView);
			}
			else
			{
				GM.invokeInMainThread([d]() {
					d->deviceContext->GenerateMips(d->shaderResourceView);
				});
			}
		}
	}
	else if (imageData.target == GMImageTarget::CubeMap)
	{
		GMComPtr<ID3D11Texture2D> texture;
		texDesc.Width = d->image->getWidth();
		texDesc.Height = d->image->getHeight();
		texDesc.MipLevels = imageData.mipLevels;
		texDesc.ArraySize = imageData.slices;
		texDesc.Format = toDxgiFormat(imageData.format);
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

		for (GMuint32 j = 0; j < texDesc.MipLevels; ++j)
		{
			GMuint32 offset = 0;
			for (GMuint32 i = 0; i < texDesc.ArraySize; ++i)
			{
				GMuint32 pitch = d->image->getWidth(j);
				resourceData[i].pSysMem = imageData.mip[j].data + offset;
				resourceData[i].SysMemPitch = pitch * imageData.channels;
				offset += d->image->getWidth(j) * d->image->getHeight(j) * imageData.channels;
			}
		}

		GM_DX_HR(d->device->CreateTexture2D(&texDesc, resourceData, &texture));
		d->resource = texture;

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = toDxgiFormat(imageData.format);
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
		srvDesc.TextureCube.MipLevels = imageData.mipLevels;
		srvDesc.TextureCube.MostDetailedMip = 0;
		GM_DX_HR(d->device->CreateShaderResourceView(
			d->resource,
			&srvDesc,
			&d->shaderResourceView
		));
	}
	else
	{
		GM_ASSERT(false);
		// 目前先只支持2D吧，之后再加
	}
	GM_delete_array(resourceData);
}

void GMDx11WhiteTexture::init()
{
	static GMbyte texData[] = { 0xFF, 0xFF, 0xFF, 0xFF };
	// 新建一个RGBA白色纹理
	D(d);
	D3D11_TEXTURE2D_DESC texDesc = { 0 };
	D3D11_SUBRESOURCE_DATA resourceData;
	GMComPtr<ID3D11Texture2D> texture;

	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	GMint32 index = 0;
	GM_ASSERT(texDesc.ArraySize == 1);
	GMuint32 pitch = 1;
	resourceData.pSysMem = texData;
	resourceData.SysMemPitch = pitch * 4;

	GM_DX_HR(d->device->CreateTexture2D(&texDesc, &resourceData, &texture));
	d->resource = texture;

	GM_ASSERT(d->resource);
	GM_DX_HR(d->device->CreateShaderResourceView(
		d->resource,
		NULL,
		&d->shaderResourceView
	));
}

void GMDx11WhiteTexture::bindSampler(GMTextureSampler*)
{
	D(d);
	if (!d->samplerState)
	{
		// 创建采样器
		D3D11_SAMPLER_DESC desc = GMDx11Helper::getDefaultSamplerDesc();
		desc.Filter = D3D11_FILTER::D3D11_FILTER_MIN_MAG_MIP_POINT;
		desc.AddressU = desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		GM_DX_HR(d->device->CreateSamplerState(&desc, &d->samplerState));
	}
}

void GMDx11EmptyTexture::init()
{
	static GMbyte texData[] = { 0 };
	D(d);
	D3D11_TEXTURE2D_DESC texDesc = { 0 };
	D3D11_SUBRESOURCE_DATA resourceData;
	GMComPtr<ID3D11Texture2D> texture;

	texDesc.Width = 1;
	texDesc.Height = 1;
	texDesc.MipLevels = 1;
	texDesc.ArraySize = 1;
	texDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	texDesc.SampleDesc.Count = 1;
	texDesc.SampleDesc.Quality = 0;
	texDesc.Usage = D3D11_USAGE_DEFAULT;
	texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	texDesc.CPUAccessFlags = 0;
	texDesc.MiscFlags = 0;

	GMint32 index = 0;
	GM_ASSERT(texDesc.ArraySize == 1);
	GMuint32 pitch = 1;
	resourceData.pSysMem = texData;
	resourceData.SysMemPitch = pitch * 4;

	GM_DX_HR(d->device->CreateTexture2D(&texDesc, &resourceData, &texture));
	d->resource = texture;

	GM_ASSERT(d->resource);
	GM_DX_HR(d->device->CreateShaderResourceView(
		d->resource,
		NULL,
		&d->shaderResourceView
	));
}