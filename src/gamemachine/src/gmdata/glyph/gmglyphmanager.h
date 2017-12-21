#ifndef __GLYPHMANAGER_H__
#define __GLYPHMANAGER_H__
#include <gmcommon.h>
#include <map>
BEGIN_NS

struct ITexture;

// 用于管理字形的类
struct GMGlyphInfo
{
	bool valid;
	GMfloat x, y; // 字形在纹理中的位置
	GMfloat width, height; // 字形在纹理中的大小
	GMfloat bearingX, bearingY;
	GMfloat advance;
};

typedef Map<GMint, Map<GMWchar, GMGlyphInfo>> CharList;

GM_PRIVATE_OBJECT(GMGlyphManager)
{
	CharList chars;
};

class GMGlyphManager : public GMObject
{
	DECLARE_PRIVATE(GMGlyphManager);

public:
	// 文字纹理的大小
	enum
	{
		CANVAS_WIDTH = 1024,
		CANVAS_HEIGHT = 1024,
	};

public:
	GMGlyphManager();
	virtual ~GMGlyphManager() {}

public:
	const GMGlyphInfo& getChar(GMWchar c, GMFontSizePt fontSize);

public:
	virtual ITexture* glyphTexture() = 0;

protected:
	virtual const GMGlyphInfo& createChar(GMWchar c, GMFontSizePt fontSize) = 0;

protected:
	GMGlyphInfo& insertChar(GMFontSizePt fontSize, GMWchar ch, const GMGlyphInfo& glyph);
	const GMGlyphInfo& getChar(GMFontSizePt fontSize, GMWchar ch);
};

END_NS
#endif
