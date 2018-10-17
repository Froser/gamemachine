#ifndef __GMDX11GLYPHMANAGER_H__
#define __GMDX11GLYPHMANAGER_H__
#include <gmcommon.h>
#include <gmglyphmanager.h>
#include <gmcom.h>
#include "gmdx11texture.h"
BEGIN_NS

GM_PRIVATE_OBJECT_UNALIGNED(GMDx11GlyphTexture)
{
	GMComPtr<ID3D11Texture2D> texture;
};

class GMDx11GlyphTexture : public GMDx11Texture
{
	GM_DECLARE_PRIVATE_NGO(GMDx11GlyphTexture)
	typedef GMDx11Texture Base;

public:
	GMDx11GlyphTexture(const IRenderContext* context);

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureIndex) override;

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
	GMAsset texture;
};

class GMDx11GlyphManager : public GMGlyphManager
{
	GM_DECLARE_PRIVATE_AND_BASE(GMDx11GlyphManager, GMGlyphManager)

public:
	using GMGlyphManager::GMGlyphManager;

public:
	virtual GMTextureAsset glyphTexture() override;
	virtual void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) override;
};

END_NS
#endif