#include "stdafx.h"
#include "gmglglyphmanager.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "utilities/assert.h"
#include "gmgltexture.h"

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

BEGIN_NS
class GMGLGlyphTexture : public ITexture
{
public:
	GMGLGlyphTexture()
	{
		glGenTextures(1, &m_id);
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexStorage2D(GL_TEXTURE_2D,
			1,
			GL_R8,
			GMGLGlyphManager::CANVAS_WIDTH,
			GMGLGlyphManager::CANVAS_HEIGHT
		);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	~GMGLGlyphTexture()
	{
		glDeleteTextures(1, &m_id);
	}

	virtual void drawTexture(TextureFrames* frames) override
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}

	GMuint getTextureId()
	{
		return m_id;
	}

private:
	GMuint m_id;
};
END_NS

GMGLGlyphManager::GMGLGlyphManager()
{
	D(d);
	d.cursor_x = d.cursor_y = 0;
	d.texture = new GMGLGlyphTexture();
}

GMGLGlyphManager::~GMGLGlyphManager()
{
	D(d);
	if (d.texture)
		delete d.texture;
}

ITexture* GMGLGlyphManager::glyphTexture()
{
	D(d);
	return d.texture;
}

const GlyphInfo& GMGLGlyphManager::createChar(GMWChar c)
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
	error = FT_Load_Char(face, c, FT_LOAD_DEFAULT);
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	error = FT_Get_Glyph(face->glyph, &glyph);
	error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

	// 创建结构
	GlyphInfo glyphInfo = { 0 };
	glyphInfo.x = d.cursor_x;
	glyphInfo.y = d.cursor_y;
	glyphInfo.width = bitmapGlyph->bitmap.width;
	glyphInfo.height = bitmapGlyph->bitmap.rows;

	d.cursor_x += bitmapGlyph->bitmap.width;
	//d.cursor_y += bitmapGlyph->bitmap.rows; //TODO

	// 创建数据
	GMuint size = bitmapGlyph->bitmap.width * bitmapGlyph->bitmap.rows;
	GMbyte* buf = new GMbyte[size];
	for (GMint j = 0; j < bitmapGlyph->bitmap.rows; j++)
	{
		for (GMint i = 0; i < bitmapGlyph->bitmap.width; i++)
		{
			buf[(i + j * bitmapGlyph->bitmap.width)] = 
				(i >= bitmapGlyph->bitmap.width || j >= bitmapGlyph->bitmap.rows) ?
				0 : bitmapGlyph->bitmap.buffer[i + bitmapGlyph->bitmap.width * j];
		}
	}

	// 创建纹理
	glBindTexture(GL_TEXTURE_2D, d.texture->getTextureId());
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 使用一个字节保存，必须设置对齐为1
	glTexSubImage2D(GL_TEXTURE_2D,
		0,
		glyphInfo.x,
		glyphInfo.y,
		glyphInfo.width,
		glyphInfo.height,
		GL_RED,
		GL_UNSIGNED_BYTE,
		buf);
	glBindTexture(GL_TEXTURE_2D, 0);
	delete[] buf;

	// 存入缓存
	CharList::_Pairib result = getCharList().insert(std::make_pair(c, glyphInfo));
	ASSERT(result.second);
	return (*(result.first)).second;
}
