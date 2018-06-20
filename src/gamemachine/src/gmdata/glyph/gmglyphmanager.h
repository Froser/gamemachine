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
	GMint x, y; // 字形在纹理中的位置
	GMint width, height; // 字形在纹理中的大小
	GMint bearingX, bearingY;
	GMint advance;
};

struct GMGlyphBitmap
{
	GMbyte* buffer;
	GMuint width;
	GMuint rows;
};

typedef void* GMFontFace;

struct GMFont
{
	GMString fontPath;
	GMFontFace face;
};

typedef HashMap<GMFontHandle, HashMap<GMint, HashMap<GMwchar, GMGlyphInfo> > > CharList;

GM_PRIVATE_OBJECT(GMGlyphManager)
{
	const IRenderContext* context = nullptr;
	CharList chars;
	GMint cursor_u, cursor_v;
	GMfloat maxHeight;
	Vector<GMFont> fonts;

	GMFontHandle defaultFontSun;
	GMFontHandle defaultFontTimesNewRoman;
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
	virtual ~GMGlyphManager();

public:
	const GMGlyphInfo& getChar(GMwchar c, GMFontSizePt fontSize, GMFontHandle font);
	GMFontHandle addFontByFileName(const GMString& fontFileName);
	GMFontHandle addFontByFullName(const GMString& fontFullName);

public:
	virtual ITexture* glyphTexture() = 0;

public:
	inline GMFontHandle getSimHei()
	{
		D(d);
		return d->defaultFontSun;
	}

	inline GMFontHandle getTimesNewRoman()
	{
		D(d);
		return d->defaultFontTimesNewRoman;
	}

private:
	virtual void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) = 0;

private:
	const GMGlyphInfo& createChar(GMwchar c, GMFontSizePt fontSize, GMFontHandle font);
	GMFont* getFont(GMFontHandle);
	GMGlyphInfo& insertChar(GMFontSizePt fontSize, GMFontHandle font, GMwchar ch, const GMGlyphInfo& glyph);
	const GMGlyphInfo& getCharInner(GMwchar c, GMFontSizePt fontSize, GMFontHandle font, GMFontHandle candidate);
};

END_NS
#endif
