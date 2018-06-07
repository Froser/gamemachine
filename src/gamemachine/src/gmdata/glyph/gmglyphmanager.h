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

struct GMGlyphBitmap
{
	GMbyte* buffer;
	GMuint width;
	GMuint rows;
};

typedef Map<GMint, Map<GMwchar, GMGlyphInfo>> CharList;

GM_PRIVATE_OBJECT(GMGlyphManager)
{
	const IRenderContext* context = nullptr;
	CharList chars;
	GMint cursor_u, cursor_v;
	GMfloat maxHeight;
};

class GMGlyphManager : public GMObject
{
	GM_DECLARE_PRIVATE(GMGlyphManager);

public:
	// 文字纹理的大小
	enum
	{
		CANVAS_WIDTH = 1024,
		CANVAS_HEIGHT = 1024,
	};

public:
	GMGlyphManager(const IRenderContext* context);
	virtual ~GMGlyphManager() {}

public:
	const GMGlyphInfo& getChar(GMwchar c, GMFontSizePt fontSize);

public:
	virtual ITexture* glyphTexture() = 0;

private:
	virtual void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) = 0;

private:
	const GMGlyphInfo& createChar(GMwchar c, GMFontSizePt fontSize);
	GMGlyphInfo& insertChar(GMFontSizePt fontSize, GMwchar ch, const GMGlyphInfo& glyph);
	const GMGlyphInfo& getChar(GMFontSizePt fontSize, GMwchar ch);
};

END_NS
#endif
