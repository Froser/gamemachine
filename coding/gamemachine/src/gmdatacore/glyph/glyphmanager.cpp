#include "stdafx.h"
#include "glyphmanager.h"

GlyphManager::GlyphManager()
{
}

const GlyphInfo GlyphManager::getChar(GMWChar c)
{
	CharList::iterator iter;
	if ((iter = m_chars.find(c)) != m_chars.end())
		return (*iter).second;
	return createChar(c);
}

GlyphManager::CharList& GlyphManager::getCharList()
{
	return m_chars;
}