#ifndef __GLYPHMANAGER_H__
#define __GLYPHMANAGER_H__
#include <gmcommon.h>
#include <map>
#include <gmassets.h>

BEGIN_NS
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

GM_PRIVATE_CLASS(GMGlyphManager);
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
	void setDefaultFontCN(GMFontHandle fontHandle);
	void setDefaultFontEN(GMFontHandle fontHandle);
	GMFontHandle getDefaultFontCN();
	GMFontHandle getDefaultFontEN();

private:
	virtual void updateTexture(const GMGlyphBitmap& bitmapGlyph, const GMGlyphInfo& glyphInfo) = 0;

protected:
	const IRenderContext* getContext();
};

END_NS
#endif
