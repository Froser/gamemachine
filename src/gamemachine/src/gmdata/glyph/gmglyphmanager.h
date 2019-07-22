#ifndef __GLYPHMANAGER_H__
#define __GLYPHMANAGER_H__
#include <gmcommon.h>
#include <map>
#include <gmassets.h>

BEGIN_NS

// 用于管理字形的类
struct GMGlyphInfo
{
	bool valid;
	GMint32 x, y; // 字形在纹理中的位置
	GMint32 width, height; // 字形在纹理中的大小
	GMint32 bearingX, bearingY;
	GMint32 advance;
};

struct GMGlyphBitmap
{
	GMbyte* buffer;
	GMuint32 width;
	GMuint32 rows;
};

typedef void* GMFontFace;

struct GMFont
{
	GMString fontPath;
	GMFontFace face;
	GMBuffer buffer;
};

typedef HashMap<GMFontHandle, HashMap<GMint32, HashMap<GMwchar, GMGlyphInfo> > > CharList;

GM_PRIVATE_OBJECT(GMGlyphManager)
{
	const IRenderContext* context = nullptr;
	CharList chars;
	GMint32 cursor_u, cursor_v;
	GMfloat maxHeight;
	Vector<GMFont> fonts;

	GMFontHandle defaultCN = GMInvalidFontHandle;
	GMFontHandle defaultEN = GMInvalidFontHandle;
};

class GM_EXPORT GMGlyphManager : public GMObject
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
	GMFontHandle addFontByMemory(GMBuffer&& buffer);

public:
	virtual GMTextureAsset glyphTexture() = 0;

public:
	void setDefaultFontCN(GMFontHandle fontHandle)
	{
		D(d);
		d->defaultCN = fontHandle;
		if (d->defaultCN == GMInvalidFontHandle)
			d->defaultCN = 0;
	}

	void setDefaultFontEN(GMFontHandle fontHandle)
	{
		D(d);
		d->defaultEN = fontHandle;
		if (d->defaultEN == GMInvalidFontHandle)
			d->defaultEN = 0;
	}

	inline GMFontHandle getDefaultFontCN()
	{
		D(d);
		return d->defaultCN;
	}

	inline GMFontHandle getDefaultFontEN()
	{
		D(d);
		return d->defaultEN;
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
