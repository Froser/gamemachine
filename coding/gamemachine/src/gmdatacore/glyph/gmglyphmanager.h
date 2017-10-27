#ifndef __GLYPHMANAGER_H__
#define __GLYPHMANAGER_H__
#include <gmcommon.h>
#include <map>
BEGIN_NS

struct ITexture;

// 用于管理字形的类
struct GlyphInfo
{
	bool valid;
	GMfloat x, y;
	GMfloat width, height;
	GMfloat bearingX, bearingY;
	GMfloat advance;
};

typedef Map<GMint, Map<GMWchar, GlyphInfo>> CharList;

GM_PRIVATE_OBJECT(GMGlyphManager)
{
	CharList chars;
};

class GMGlyphManager : public GMObject
{
	DECLARE_PRIVATE(GMGlyphManager);

public:
	GMGlyphManager();
	virtual ~GMGlyphManager() {}

public:
	const GlyphInfo getChar(GMWchar c, GMint fontSize);

public:
	virtual ITexture* glyphTexture() = 0;

protected:
	virtual const GlyphInfo& createChar(GMWchar c, GMint fontSize) = 0;

protected:
	GlyphInfo& insertChar(GMint fontSize, GMWchar ch, const GlyphInfo& glyph);
	const GlyphInfo& getChar(GMint fontSize, GMWchar ch);
};

END_NS
#endif
