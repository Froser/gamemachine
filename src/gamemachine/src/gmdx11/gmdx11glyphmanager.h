#ifndef __GMDX11GLYPHMANAGER_H__
#define __GMDX11GLYPHMANAGER_H__
#include <gmcommon.h>
#include <gmglyphmanager.h>
#include <gmcom.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11GlyphTexture)
{
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMComPtr<ID3D11Device> device;
	GMComPtr<ID3D11ShaderResourceView> resourceView;
	GMComPtr<ID3D11Texture2D> texture;
	GMComPtr<ID3D11SamplerState> samplerState;
};

class GMDx11GlyphTexture : public ITexture
{
	DECLARE_PRIVATE(GMDx11GlyphTexture)

public:
	GMDx11GlyphTexture();

public:
	virtual void init() override;
	virtual void drawTexture(GMTextureFrames* frames, GMint textureIndex) override;

public:
	inline ID3D11Texture2D* getD3D11Texture()
	{
		D(d);
		return d->texture;
	}
};

GM_PRIVATE_OBJECT(GMDx11GlyphManager)
{
	GMComPtr<ID3D11DeviceContext> deviceContext;
	GMScopePtr<GMDx11GlyphTexture> texture;
};

class GMDx11GlyphManager : public GMGlyphManager
{
	DECLARE_PRIVATE(GMDx11GlyphManager)

public:
	GMDx11GlyphManager();

public:
	virtual ITexture* glyphTexture() override;
	virtual void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) override;
};

END_NS
#endif