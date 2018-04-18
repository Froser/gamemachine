#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11glyphmanager.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "foundation/utilities/tools.h"
#include "gmdxincludes.h"
#include "gmdx11helper.h"

GMDx11GlyphTexture::GMDx11GlyphTexture()
{
	D(d);
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&d->device);
	GM_ASSERT(d->device);
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
	GM_ASSERT(d->deviceContext);
}

GMDx11GlyphManager::GMDx11GlyphManager()
{
}

ITexture* GMDx11GlyphManager::glyphTexture()
{
	D(d);
	if (!d->texture)
	{
		d->texture.reset(new GMDx11GlyphTexture());
		d->texture->init();
	}
	return d->texture;
}

void GMDx11GlyphManager::updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo)
{
	D(d);
	if (!d->deviceContext)
	{
		GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
		GM_ASSERT(d->deviceContext);
	}

	D3D11_BOX box = {
		(UINT)glyphInfo.x, //left
		(UINT)glyphInfo.y, //top
		0, //front
		(UINT)(glyphInfo.x + glyphInfo.width), //right
		(UINT)(glyphInfo.y + glyphInfo.height), //bottom
		1 //back
	};
	d->deviceContext->UpdateSubresource(
		d->texture->getD3D11Texture(),
		0,
		&box,
		bitmapGlyph.buffer,
		bitmapGlyph.width,
		0
	);
}

void GMDx11GlyphTexture::init()
{
	D(d);
	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = GMDx11GlyphManager::CANVAS_WIDTH;
	desc.Height = GMDx11GlyphManager::CANVAS_HEIGHT;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA resourceData = { 0 };
	resourceData.pSysMem = nullptr;
	resourceData.SysMemPitch = GMDx11GlyphManager::CANVAS_WIDTH; //每个字符占1个字节

	GM_DX_HR(d->device->CreateTexture2D(&desc, 0, &d->texture));

	GM_DX11_SET_OBJECT_NAME_A(d->texture, "GM_GLYPH_TEXTURE");
	GM_DX_HR(d->device->CreateShaderResourceView(
		d->texture,
		NULL,
		&d->resourceView
	));
}

void GMDx11GlyphTexture::drawTexture(GMTextureFrames* frames, GMint textureIndex)
{
	D(d);
	if (!d->samplerState)
	{
		// 创建采样器
		D3D11_SAMPLER_DESC desc = GMDx11Helper::GMGetDx11DefaultSamplerDesc();
		desc.Filter = GMDx11Helper::GMGetDx11Filter(frames->getMinFilter(), frames->getMagFilter());
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		GM_DX_HR(d->device->CreateSamplerState(&desc, &d->samplerState));
	}

	GM_ASSERT(d->samplerState);
	d->deviceContext->PSSetShaderResources(0, 1, &d->resourceView);
	d->deviceContext->PSSetSamplers(0, 1, &d->samplerState);
}