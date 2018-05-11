#include "stdafx.h"
#include "gmdx11texture.h"
#include <gamemachine.h>
#include "gmdx11graphic_engine.h"
#include "gmdx11helper.h"

namespace
{
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
		case GMS_Wrap::CLAMP_TO_BORDER:
			return D3D11_TEXTURE_ADDRESS_BORDER;
		case GMS_Wrap::CLAMP_TO_EDGE:
			return D3D11_TEXTURE_ADDRESS_CLAMP;
		case GMS_Wrap::REPEAT:
			return D3D11_TEXTURE_ADDRESS_WRAP;
		case GMS_Wrap::MIRRORED_REPEAT:
			return D3D11_TEXTURE_ADDRESS_MIRROR;
		default:
			GM_ASSERT(false);
			return D3D11_TEXTURE_ADDRESS_WRAP;
		}
	}
}

GMDx11Texture::GMDx11Texture(GMImage* image)
{
	D(d);
	d->image = image;
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&d->device);
	GM_ASSERT(d->device);

	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
	GM_ASSERT(d->deviceContext);
}

GMDx11Texture::~GMDx11Texture()
{
}

void GMDx11Texture::useTexture(GMTextureFrames* frames, GMint textureIndex)
{
	D(d);
	if (!d->samplerState)
	{
		// 创建采样器
		D3D11_SAMPLER_DESC desc = GMDx11Helper::GMGetDx11DefaultSamplerDesc();
		if (frames)
		{
			desc.Filter = GMDx11Helper::GMGetDx11Filter(frames->getMinFilter(), frames->getMagFilter());
			desc.AddressU = getAddressMode(frames->getWrapS());
			desc.AddressV = getAddressMode(frames->getWrapT());
		}
		GM_DX_HR(d->device->CreateSamplerState(&desc, &d->samplerState));
	}
	
	GM_ASSERT(d->samplerState);
	d->deviceContext->PSSetShaderResources(textureIndex, 1, &d->shaderResourceView);
	d->deviceContext->PSSetSamplers(textureIndex, 1, &d->samplerState);
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
		texDesc.MipLevels = imageData.mipLevels;
		texDesc.ArraySize = imageData.slices;
		texDesc.Format = toDxgiFormat(imageData.format);
		texDesc.SampleDesc.Count = 1;
		texDesc.SampleDesc.Quality = 0;
		texDesc.Usage = D3D11_USAGE_DEFAULT;
		texDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		texDesc.CPUAccessFlags = 0;
		texDesc.MiscFlags = 0;

		GMint index = 0;
		GM_ASSERT(texDesc.ArraySize == 1);
		for (GMuint j = 0; j < texDesc.MipLevels; ++j, ++index)
		{
			GMuint pitch = d->image->getWidth(j);
			resourceData[index].pSysMem = imageData.mip[j].data;
			resourceData[index].SysMemPitch = pitch * d->image->getData().channels;
		}

		GM_DX_HR(d->device->CreateTexture2D(&texDesc, resourceData, &texture));
		d->resource = texture;

		GM_ASSERT(d->resource);
		GM_DX_HR(d->device->CreateShaderResourceView(
			d->resource,
			NULL,
			&d->shaderResourceView
		));
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

		for (GMuint j = 0; j < texDesc.MipLevels; ++j)
		{
			GMuint offset = 0;
			for (GMuint i = 0; i < texDesc.ArraySize; ++i)
			{
				GMuint pitch = d->image->getWidth(j);
				resourceData[i].pSysMem = imageData.mip[j].data + offset;
				resourceData[i].SysMemPitch = pitch * imageData.channels;
				offset += d->image->getWidth(j) * d->image->getHeight(j) * imageData.channels;
			}
		}

		GM_DX_HR(d->device->CreateTexture2D(&texDesc, resourceData, &texture));
		d->resource = texture;

		GM_delete_array(resourceData);
		GM_ASSERT(d->resource);

		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		if (imageData.target == GMImageTarget::CubeMap)
		{
			srvDesc.Format = toDxgiFormat(imageData.format);
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = texDesc.MipLevels;
			srvDesc.TextureCube.MostDetailedMip = 0;
		}
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