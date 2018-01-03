#include "stdafx.h"
#include "gmglyphmanager.h"

GMGlyphManager::GMGlyphManager()
{
}

const GMGlyphInfo& GMGlyphManager::getChar(GMwchar c, GMint fontSize)
{
	D(d);
	CharList::mapped_type::iterator iter;
	auto& charsWithSize = d->chars[fontSize];
	if ((iter = charsWithSize.find(c)) != charsWithSize.end())
		return (*iter).second;
	return createChar(c, fontSize);
}

GMGlyphInfo& GMGlyphManager::insertChar(GMint fontSize, GMwchar ch, const GMGlyphInfo& glyph)
{
	D(d);
	auto result = d->chars[fontSize].insert({ ch, glyph });
	GM_ASSERT(result.second);
	return (*(result.first)).second;
}

const GMGlyphInfo& GMGlyphManager::getChar(GMint fontSize, GMwchar ch)
{
	D(d);
	return d->chars[fontSize][ch];
}