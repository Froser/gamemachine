#ifndef __GLYPHPAINTER_H__
#define __GLYPHPAINTER_H__
#include "common.h"

BEGIN_NS
struct FontAttributes
{
	const char* fontFile;
	GMuint charHeight;
	GMuint horzResolution, vertResolution;
};

// 专门用于绘制文字以及排版的一个类
struct GlyphPainterPrivate;
class Image;
class GlyphPainter
{
	DEFINE_PRIVATE_ON_HEAP(GlyphPainter);

public:
	GlyphPainter();
	~GlyphPainter();

public:
	void drawString(const FontAttributes& attrs, const wchar_t* content);
	void getImage(OUT Image** image);

private:
	void createImage();
	void copyBuffer();
};

END_NS
#endif