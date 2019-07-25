#ifndef __GMGLGLYPHMANAGER_H__
#define __GMGLGLYPHMANAGER_H__
#include <gmcommon.h>
#include "gmdata/glyph/gmglyphmanager.h"
BEGIN_NS

class GMGLGlyphTexture;
GM_PRIVATE_CLASS(GMGLGlyphManager);
class GMGLGlyphManager : public GMGlyphManager
{
	GM_DECLARE_PRIVATE(GMGLGlyphManager);

public:
	GMGLGlyphManager(const IRenderContext* context);
	~GMGLGlyphManager();

public:
	virtual GMTextureAsset glyphTexture() override;

protected:
	void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) override;
};

END_NS
#endif