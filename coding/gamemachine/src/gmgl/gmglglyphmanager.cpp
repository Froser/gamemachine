#include "stdafx.h"
#include "gmglglyphmanager.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"
#include "gmgltexture.h"

#if _MSC_VER
#	include <shlobj.h>
#else
//TODO 这里在其他系统下存在问题
BOOL SHGetSpecialFolderPathA(HWND hwnd, LPSTR pszPath, int csidl, BOOL fCreate);
BOOL SHGetSpecialFolderPathW(HWND hwnd, LPWSTR pszPath, int csidl, BOOL fCreate);
#	ifdef UNICODE
#		define SHGetSpecialFolderPath  SHGetSpecialFolderPathW
#	else
#		define SHGetSpecialFolderPath  SHGetSpecialFolderPathA
#	endif // !UNICODE
#	define CSIDL_FONTS 0x0014
#endif

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

// 从系统获取字体文件，从上到下遍历，越靠前优先级越高
struct FontList
{
	char fontName[MAX_PATH];
};
static FontList fontNameList[] = {
	{ "msyh.ttf" },
	{ "times.ttf" }
};
static GMuint fontNameNum = 2;

static FT_Error loadFace(FT_Face* face)
{
	FT_Error err = FT_Err_Cannot_Open_Resource;
#if _WINDOWS
	GMWchar path[MAX_PATH];
	SHGetSpecialFolderPath(NULL, path, CSIDL_FONTS, FALSE);

	std::string strPath;
	for (GMuint i = 0; i < fontNameNum; i++)
	{
		GMString p(path);
		p.append(_L("/"));
		p.append(fontNameList[i].fontName);
		strPath = p.toStdString();
		err = FT_New_Face(g_lib.library, strPath.c_str(), 0, face);
		if (err == FT_Err_Ok)
			break;
	}
#endif
	return err;
}

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

	virtual void drawTexture(GMTextureFrames* frames) override
	{
		glBindTexture(GL_TEXTURE_2D, m_id);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
	d->cursor_u = d->cursor_v = 0;
	d->maxHeight = 0;
	d->texture = new GMGLGlyphTexture();
}

GMGLGlyphManager::~GMGLGlyphManager()
{
	D(d);
	if (d->texture)
		delete d->texture;
}

ITexture* GMGLGlyphManager::glyphTexture()
{
	D(d);
	return d->texture;
}

const GMGlyphInfo& GMGLGlyphManager::createChar(GMWchar c, GMint fontSize)
{
	D(d);
	static GMGlyphInfo errGlyph = { false };
	FT_Error error;
	FT_Face face;
	FT_Glyph glyph;
	error = loadFace(&face);
	if (error != FT_Err_Ok)
	{
		gm_error(_L("cannot found font file, cannot draw characters."));
		return errGlyph;
	}

	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	error = FT_Set_Char_Size(face, 0, fontSize << 6, GMScreen::dpi(), GMScreen::dpi());
	error = FT_Load_Glyph(face, FT_Get_Char_Index(face, c), FT_LOAD_DEFAULT);
	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	error = FT_Get_Glyph(face->glyph, &glyph);
	error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

	// 创建结构
	if (d->cursor_u + bitmapGlyph->bitmap.width > CANVAS_WIDTH)
	{
		d->cursor_v += d->maxHeight + 1;
		d->maxHeight = 0;
		d->cursor_u = 0;
		if (d->cursor_v + bitmapGlyph->bitmap.rows > CANVAS_HEIGHT)
		{
			gm_error(_L("no texture space for glyph!"));
			GM_ASSERT(false);
		}
	}

	GMGlyphInfo glyphInfo = { 0 };
	glyphInfo.valid = true;
	glyphInfo.x = d->cursor_u;
	glyphInfo.y = d->cursor_v;
	glyphInfo.width = face->glyph->metrics.width >> 6;
	glyphInfo.height = face->glyph->metrics.height >> 6;
	glyphInfo.bearingX = face->glyph->metrics.horiBearingX >> 6;
	glyphInfo.bearingY = face->glyph->metrics.horiBearingY >> 6;
	glyphInfo.advance = face->glyph->metrics.horiAdvance >> 6;

	if (d->maxHeight < glyphInfo.height)
		d->maxHeight = glyphInfo.height;
	d->cursor_u += bitmapGlyph->bitmap.width + 1;

	// 创建纹理
	glBindTexture(GL_TEXTURE_2D, d->texture->getTextureId());
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // 使用一个字节保存，必须设置对齐为1
	glTexSubImage2D(GL_TEXTURE_2D,
		0,
		glyphInfo.x,
		glyphInfo.y,
		glyphInfo.width,
		glyphInfo.height,
		GL_RED,
		GL_UNSIGNED_BYTE,
		bitmapGlyph->bitmap.buffer);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 释放资源
	FT_Done_Glyph(glyph);

	// 存入缓存
	return insertChar(fontSize, c, glyphInfo);
}