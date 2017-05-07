#include "stdafx.h"
#include "gmglglyphmanager.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "utilities/autoptr.h"

#define FONT_SIZE 12
#define RESOLUTION 72
#define CANVAS_WIDTH FONT_SIZE * 100
#define CANVAS_HEIGHT FONT_SIZE * 100

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

GMGLGlyphManager::GMGLGlyphManager()
{
	D(d);

	glGenTextures(1, &d.textureId);
	glBindTexture(GL_TEXTURE_2D, d.textureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexStorage2D(GL_TEXTURE_2D,
		0,
		GL_LUMINANCE_ALPHA,
		CANVAS_WIDTH,
		CANVAS_HEIGHT
		);
	glBindTexture(GL_TEXTURE_2D, 0);
}

GlyphInfo GMGLGlyphManager::createChar(GMWChar c)
{
	D(d);
	// 先获取字形
	FT_Error error;
	//	FT_UInt glyphIndex;
	FT_Face face;
	FT_Glyph glyph;
	error = FT_New_Face(g_lib.library, "C:\\Windows\\Fonts\\simsunb.TTF", 0, &face);
	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	error = FT_Set_Char_Size(face, 0, FONT_SIZE << 6, RESOLUTION, RESOLUTION);
	//glyphIndex = FT_Get_Char_Index(face, 'a');
	//error = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
	error = FT_Load_Char(face, L'a', FT_LOAD_DEFAULT);
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	error = FT_Get_Glyph(face->glyph, &glyph);
	error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

	// 创建结构
	GlyphInfo glyphInfo;
	glyphInfo.x = d.cursor_x;
	glyphInfo.y = d.cursor_y;
	glyphInfo.width = bitmapGlyph->bitmap.width;
	glyphInfo.height = bitmapGlyph->bitmap.rows;

	d.cursor_x += bitmapGlyph->bitmap.width;
	d.cursor_y += bitmapGlyph->bitmap.rows; //TODO

	// 创建数据
	GMuint size = bitmapGlyph->bitmap.width * bitmapGlyph->bitmap.rows * 2;
	AutoPtr<GMbyte> buf;
	buf.reset(new GMbyte[size]);
	for (GMint j = 0; j < bitmapGlyph->bitmap.rows; j++)
	{
		for (GMint i = 0; i < bitmapGlyph->bitmap.width; i++)
		{
			buf[2 * (i + (bitmapGlyph->bitmap.rows - j - 1) * bitmapGlyph->bitmap.width)] = buf[2 * (i + (bitmapGlyph->bitmap.rows - j - 1) * bitmapGlyph->bitmap.width) + 1] =
				(i >= bitmapGlyph->bitmap.width || j >= bitmapGlyph->bitmap.rows) ?
				0 : bitmapGlyph->bitmap.buffer[i + bitmapGlyph->bitmap.width * j];
		}
	}

	// 创建纹理
	glBindTexture(GL_TEXTURE_2D, d.textureId);
	glTexSubImage2D(GL_TEXTURE_2D,
		0,
		glyphInfo.x,
		glyphInfo.y,
		glyphInfo.width,
		glyphInfo.height,
		GL_LUMINANCE_ALPHA,
		GL_UNSIGNED_BYTE,
		buf);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 存入缓存
	getCharList().insert(std::make_pair(c, glyphInfo));
	return glyphInfo;
}
