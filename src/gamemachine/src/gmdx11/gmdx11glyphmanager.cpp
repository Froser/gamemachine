#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11glyphmanager.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "foundation/utilities/tools.h"
#include "gmdxincludes.h"
#include "gmdx11helper.h"
#include "gmdx11texture_p.h"

BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMDx11GlyphManager)
{
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMAsset texture;
};

GMDx11GlyphManager::GMDx11GlyphManager(const IRenderContext* context)
	: Base(context)
{
	GM_CREATE_DATA();
}

GMDx11GlyphManager::~GMDx11GlyphManager()
{

}

GMTextureAsset GMDx11GlyphManager::glyphTexture()
{
	D(d);
	if (d->texture.isEmpty())
	{
		d->texture = GMAsset(GMAssetType::Texture, new GMDx11GlyphTexture(getContext()));
		d->texture.getTexture()->init();
	}
	return d->texture;
}

void GMDx11GlyphManager::updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo)
{
	D(d);
	if (!d->deviceContext)
	{
		getContext()->getEngine()->getInterface(GameMachineInterfaceID::D3D11DeviceContext, (void**)&d->deviceContext);
		GM_ASSERT(d->deviceContext);
	}

	if (d->texture.isEmpty())
		glyphTexture();

	D3D11_BOX box = {
		(UINT)glyphInfo.x, //left
		(UINT)glyphInfo.y, //top
		0, //front
		(UINT)(glyphInfo.x + glyphInfo.width), //right
		(UINT)(glyphInfo.y + glyphInfo.height), //bottom
		1 //back
	};
	
	GMDx11GlyphTexture* texture = d->texture.get<GMDx11GlyphTexture*>();
	d->deviceContext->UpdateSubresource(
		texture->getD3D11Texture(),
		0,
		&box,
		bitmapGlyph.buffer,
		bitmapGlyph.width,
		0
	);
}

GM_PRIVATE_OBJECT_UNALIGNED(GMDx11GlyphTexture)
{
	GMComPtr<ID3D11Texture2D> texture;
};

GMDx11GlyphTexture::GMDx11GlyphTexture(const IRenderContext* context)
	: GMDx11Texture(context, nullptr)
{
	GM_CREATE_DATA();
}

void GMDx11GlyphTexture::init()
{
	D(d);
	D_BASE(db, Base);
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

	GM_DX_HR(db->device->CreateTexture2D(&desc, 0, &d->texture));

	GM_DX11_SET_OBJECT_NAME_A(d->texture, "GM_GLYPH_TEXTURE");
	GM_DX_HR(db->device->CreateShaderResourceView(
		d->texture,
		NULL,
		&db->shaderResourceView
	));
}

void GMDx11GlyphTexture::bindSampler(GMTextureSampler* sampler)
{
	D(d);
	D_BASE(db, Base);
	if (!db->samplerState)
	{
		// 创建采样器
		D3D11_SAMPLER_DESC desc = GMDx11Helper::getDefaultSamplerDesc();
		desc.Filter = GMDx11Helper::getTextureFilter(sampler->getMinFilter(), sampler->getMagFilter());
		desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
		GM_DX_HR(db->device->CreateSamplerState(&desc, &db->samplerState));
	}
}

void GMDx11GlyphTexture::useTexture(GMint32)
{
	D(d);
	Base::useTexture((GMint32)GMTextureType::Ambient);
}

ID3D11Texture2D* GMDx11GlyphTexture::getD3D11Texture()
{
	D(d);
	return d->texture;
}

END_NS