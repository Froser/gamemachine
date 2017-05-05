#include "stdafx.h"
#include "utilities/autoptr.h"
#include "glyphpainter.h"
#include "gmdatacore/image.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "utilities/assert.h"

BEGIN_NS
struct TypoLibrary
{
	TypoLibrary()
	{
		FT_Init_FreeType(&library);
	}

	~TypoLibrary()
	{
		FT_Done_FreeType(library);
	}

	FT_Library library;
};

static TypoLibrary g_lib;

struct GlyphPainterPrivate
{
	AutoPtr<Image> image;
	FT_BitmapGlyph glyph;
};
END_NS

GlyphPainter::GlyphPainter()
{
	D(d);
	d = new DataType;
}

GlyphPainter::~GlyphPainter()
{
	D(d);
	delete d;
}

void GlyphPainter::createImage()
{
	D(d);
	d->image.reset(new Image());
	ImageData& imgData = d->image->getData();
	imgData.target = GL_TEXTURE_2D;
	imgData.mipLevels = 1;
	imgData.internalFormat = GL_RGB16;
	imgData.format = GL_LUMINANCE_ALPHA;

	imgData.swizzle[0] = GL_RED;
	imgData.swizzle[1] = GL_GREEN;
	imgData.swizzle[2] = GL_BLUE;
	imgData.swizzle[3] = GL_ALPHA;
	imgData.type = GL_UNSIGNED_BYTE;
	imgData.mip[0].height = d->glyph->bitmap.rows;
	imgData.mip[0].width = d->glyph->bitmap.width;
}

void GlyphPainter::copyBuffer()
{
	D(d);
	const FT_Bitmap& bitmap = d->glyph->bitmap;
	ImageData& imgData = d->image->getData();

	GMuint size = bitmap.width * bitmap.rows * 2;
	imgData.mip[0].data = new GMbyte[size];
	imgData.size = size;

	GMbyte* buf = imgData.mip[0].data;
	for (GMint j = 0; j < bitmap.rows; j++)
	{
		for (GMint i = 0; i < bitmap.width; i++)
		{
			buf[2 * (i + (bitmap.rows - j - 1) * bitmap.width)] = imgData.mip[0].data[2 * (i + (bitmap.rows - j - 1) * bitmap.width) + 1] =
				(i >= bitmap.width || j >= bitmap.rows) ?
				0 : bitmap.buffer[i + bitmap.width * j];
		}
	}
}

void GlyphPainter::drawString(const FontAttributes& attrs, const wchar_t* content)
{
	D(d);
	FT_Error error;
//	FT_UInt glyphIndex;
	FT_Face face;
	FT_Glyph glyph;
	error = FT_New_Face(g_lib.library, "C:\\Windows\\Fonts\\simsunb.TTF", 0, &face);
	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	error = FT_Set_Char_Size(face, 0, attrs.charHeight << 6, attrs.horzResolution, attrs.vertResolution);
	//glyphIndex = FT_Get_Char_Index(face, 'a');
	//error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
	error = FT_Load_Char(face, L'a', FT_LOAD_DEFAULT);
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	error = FT_Get_Glyph(face->glyph, &glyph);
	error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	d->glyph = (FT_BitmapGlyph)glyph;

	createImage();
	copyBuffer();
}

void GlyphPainter::getImage(OUT Image** image)
{
	D(d);
	Image* img = new Image();
	memcpy(&img->getData(), &d->image->getData(), sizeof(d->image->getData()));

	img->getData().mip[0].data = new GMbyte[d->image->getData().size];
	memcpy(img->getData().mip[0].data, d->image->getData().mip[0].data, d->image->getData().size);

	*image = img;
}