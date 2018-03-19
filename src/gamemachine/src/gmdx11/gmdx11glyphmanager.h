#ifndef __GMDX11GLYPHMANAGER_H__
#define __GMDX11GLYPHMANAGER_H__
#include <gmcommon.h>
#include <gmglyphmanager.h>
BEGIN_NS

class GMDx11GlyphTexture
{

};

GM_PRIVATE_OBJECT(GMDx11GlyphManager)
{
};

class GMDx11GlyphManager : public GMGlyphManager
{
	DECLARE_PRIVATE(GMDx11GlyphManager)

public:
	GMDx11GlyphManager();

public:
	virtual ITexture* glyphTexture() override;
	virtual void createTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) override;
};

END_NS
#endif