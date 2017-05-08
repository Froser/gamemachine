#ifndef __GLYPHMANAGER_H__
#define __GLYPHMANAGER_H__
#include "common.h"
#include <map>
BEGIN_NS

struct ITexture;

// 用于管理字形的类
struct GlyphInfo
{
	GMfloat x, y;
	GMfloat width, height;
};

class GlyphManager
{
	typedef std::map<GMWChar, GlyphInfo> CharList;

public:
	GlyphManager();

public:
	const GlyphInfo getChar(GMWChar c);

public:
	virtual ITexture* glyphTexture() = 0;

protected:
	virtual const GlyphInfo& createChar(GMWChar c) = 0;

protected:
	CharList& getCharList();

private:
	CharList m_chars;
};

END_NS
#endif