#include "stdafx.h"
#include "gmglyphmanager.h"

GMGlyphManager::GMGlyphManager()
{
}

const GMGlyphInfo& GMGlyphManager::getChar(GMWchar c, GMint fontSize)
{
	D(d);
	CharList::mapped_type::iterator iter;
	auto& charsWithSize = d->chars[fontSize];
	if ((iter = charsWithSize.find(c)) != charsWithSize.end())
		return (*iter).second;
	return createChar(c, fontSize);
}

GMGlyphInfo& GMGlyphManager::insertChar(GMint fontSize, GMWchar ch, const GMGlyphInfo& glyph)
{
	D(d);
	auto& result = d->chars[fontSize].insert({ ch, glyph });
	GM_ASSERT(result.second);
	return (*(result.first)).second;
}

const GMGlyphInfo& GMGlyphManager::getChar(GMint fontSize, GMWchar ch)
{
	D(d);
	return d->chars[fontSize][ch];
}