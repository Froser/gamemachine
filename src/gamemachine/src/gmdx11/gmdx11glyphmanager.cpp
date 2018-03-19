#include "stdafx.h"
#include <gamemachine.h>
#include "gmdx11glyphmanager.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "foundation/utilities/tools.h"
#include "gmdxincludes.h"

GMDx11GlyphTexture::GMDx11GlyphTexture()
{
	D(d);
	GM.getGraphicEngine()->getInterface(GameMachineInterfaceID::D3D11Device, (void**)&d->device);
	GM_ASSERT(d->device);

	D3D11_TEXTURE2D_DESC desc = { 0 };
	desc.Width = GMDx11GlyphManager::CANVAS_WIDTH;
	desc.Height = GMDx11GlyphManager::CANVAS_HEIGHT;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;

	D3D11_SUBRESOURCE_DATA resourceData;
	resourceData.pSysMem = nullptr;
	resourceData.SysMemPitch = GMDx11GlyphManager::CANVAS_WIDTH; //每个字符占1个字节

	GM_DX_HR(d->device->CreateTexture2D(&desc, &resourceData, &d->texture));
}

GMDx11GlyphManager::GMDx11GlyphManager()
{

}

ITexture* GMDx11GlyphManager::glyphTexture()
{
	D(d);
	if (!d->texture)
		d->texture.reset(new GMDx11GlyphTexture());
	return d->texture;
}

void GMDx11GlyphManager::createTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo)
{

}

void GMDx11GlyphTexture::drawTexture(GMTextureFrames* frames)
{
}
