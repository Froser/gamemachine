#include "stdafx.h"
#include "gmdx11glyphmanager.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "foundation/utilities/tools.h"

GMDx11GlyphManager::GMDx11GlyphManager()
{

}

ITexture* GMDx11GlyphManager::glyphTexture()
{
	return nullptr;
}

void GMDx11GlyphManager::createTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo)
{
}
