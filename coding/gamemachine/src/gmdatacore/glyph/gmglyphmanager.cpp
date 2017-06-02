#include "stdafx.h"
#include "gmglyphmanager.h"

GMGlyphManager::GMGlyphManager()
{
}

const GlyphInfo GMGlyphManager::getChar(GMWChar c)
{
	CharList::iterator iter;
	if ((iter = m_chars.find(c)) != m_chars.end())
		return (*iter).second;
	return createChar(c);
}

GMGlyphManager::CharList& GMGlyphManager::getCharList()
{
	return m_chars;
}