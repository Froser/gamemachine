#ifndef __GMDX11GLYPHMANAGER_H__
#define __GMDX11GLYPHMANAGER_H__
#include <gmcommon.h>
#include <gmglyphmanager.h>
#include <gmcom.h>
#include "gmdx11texture.h"
BEGIN_NS

GM_PRIVATE_CLASS(GMDx11GlyphTexture);
class GMDx11GlyphTexture : public GMDx11Texture
{
	GM_DECLARE_PRIVATE(GMDx11GlyphTexture)
	GM_DECLARE_BASE(GMDx11Texture)

public:
	GMDx11GlyphTexture(const IRenderContext* context);

public:
	virtual void init() override;
	virtual void bindSampler(GMTextureSampler* sampler) override;
	virtual void useTexture(GMint32 textureIndex) override;

public:
	ID3D11Texture2D* getD3D11Texture();
};

GM_PRIVATE_CLASS(GMDx11GlyphManager);
class GMDx11GlyphManager : public GMGlyphManager
{
	GM_DECLARE_PRIVATE(GMDx11GlyphManager)
	GM_DECLARE_BASE(GMGlyphManager)

public:
	GMDx11GlyphManager(const IRenderContext* context);
	~GMDx11GlyphManager();

public:
	virtual GMTextureAsset glyphTexture() override;
	virtual void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) override;
};

END_NS
#endif