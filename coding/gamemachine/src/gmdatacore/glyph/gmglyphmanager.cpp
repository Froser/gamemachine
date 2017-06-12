#include "stdafx.h"
#include "gmglyphmanager.h"

GMGlyphManager::GMGlyphManager()
{
}

const GlyphInfo GMGlyphManager::getChar(GMWchar c)
{
	D(d);
	CharList::iterator iter;
	if ((iter = d->chars.find(c)) != d->chars.end())
		return (*iter).second;
	return createChar(c);
}

CharList& GMGlyphManager::getCharList()
{
	D(d);
	return d->chars;
}