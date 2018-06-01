#ifndef __GMDX11GLYPHMANAGER_H__
#define __GMDX11GLYPHMANAGER_H__
#include <gmcommon.h>
#include <gmglyphmanager.h>
#include <gmcom.h>
#include "gmdx11texture.h"
BEGIN_NS

GM_PRIVATE_OBJECT(GMDx11GlyphTexture)
{
	GMComPtr<ID3D11Texture2D> texture;
};

class GMDx11GlyphTexture : public GMDx11Texture
{
	DECLARE_PRIVATE_AND_BASE(GMDx11GlyphTexture, GMDx11Texture)

public:
	GMDx11GlyphTexture(const GMContext* context);

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint textureIndex) override;

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
	DECLARE_PRIVATE_AND_BASE(GMDx11GlyphManager, GMGlyphManager)

public:
	using GMGlyphManager::GMGlyphManager;

public:
	virtual ITexture* glyphTexture() override;
	virtual void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) override;
};

END_NS
#endif