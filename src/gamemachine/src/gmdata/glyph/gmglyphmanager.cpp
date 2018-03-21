#include "stdafx.h"
#include "gmglyphmanager.h"
#include "foundation/utilities/tools.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

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

namespace
{
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
		GMwchar fontName[MAX_PATH];
	};

	FontList fontNameList[] = {
		{ L"msyh.ttf" },
		{ L"times.ttf" }
	};
	GMuint fontNameNum = 2;

	FT_Error loadFace(FT_Face* face)
	{
		FT_Error err = FT_Err_Cannot_Open_Resource;
#if GM_WINDOWS
		GMwchar path[MAX_PATH];
		SHGetSpecialFolderPath(NULL, path, CSIDL_FONTS, FALSE);

		std::string strPath;
		for (GMuint i = 0; i < fontNameNum; i++)
		{
			GMString p(path);
			p.append(L"/");
			p.append(fontNameList[i].fontName);
			strPath = p.toStdString();
			err = FT_New_Face(g_lib.library, strPath.c_str(), 0, face);
			if (err == FT_Err_Ok)
				break;
		}
#endif
		return err;
	}
}

const GMGlyphInfo& GMGlyphManager::getChar(GMwchar c, GMint fontSize)
{
	D(d);
	CharList::mapped_type::iterator iter;
	auto& charsWithSize = d->chars[fontSize];
	if ((iter = charsWithSize.find(c)) != charsWithSize.end())
		return (*iter).second;
	return createChar(c, fontSize);
}

GMGlyphInfo& GMGlyphManager::insertChar(GMint fontSize, GMwchar ch, const GMGlyphInfo& glyph)
{
	D(d);
	auto result = d->chars[fontSize].insert({ ch, glyph });
	GM_ASSERT(result.second);
	return (*(result.first)).second;
}

const GMGlyphInfo& GMGlyphManager::getChar(GMint fontSize, GMwchar ch)
{
	D(d);
	return d->chars[fontSize][ch];
}

const GMGlyphInfo& GMGlyphManager::createChar(GMwchar c, GMFontSizePt fontSize)
{
	D(d);
	static GMGlyphInfo errGlyph = { false };
	FT_Error error;
	FT_Face face;
	FT_Glyph glyph;
	error = loadFace(&face);
	if (error != FT_Err_Ok)
	{
		gm_error(L"cannot found font file, cannot draw characters.");
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
			gm_error(L"no texture space for glyph!");
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
	GMGlyphBitmap bitmap = {
		bitmapGlyph->bitmap.buffer,
		bitmapGlyph->bitmap.width,
		bitmapGlyph->bitmap.rows
	};
	updateTexture(bitmap, glyphInfo);

	// 释放资源
	FT_Done_Glyph(glyph);

	return insertChar(fontSize, c, glyphInfo);
}