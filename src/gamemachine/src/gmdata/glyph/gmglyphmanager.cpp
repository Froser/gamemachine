#include "stdafx.h"
#include "gmglyphmanager.h"
#include "foundation/utilities/tools.h"
#include "ft2build.h"
#include "freetype/freetype.h"
#include "freetype/ftglyph.h"

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

	FT_Error loadFace(const GMString& fontFullPath, FT_Face* face)
	{
		FT_Error err = FT_New_Face(g_lib.library, fontFullPath.toStdString().c_str(), 0, face);
		return err;
	}
}

const GMGlyphInfo& GMGlyphManager::getChar(GMwchar c, GMint fontSize, GMFontHandle font)
{
	return getCharInner(c, fontSize, font, 0);
}

const GMGlyphInfo& GMGlyphManager::getCharInner(GMwchar c, GMFontSizePt fontSize, GMFontHandle font, GMFontHandle candidate)
{
	D(d);
	static const GMGlyphInfo err = { false };
	if (font >= d->fonts.size())
		return err;

	auto& charsWithSize = d->chars[font][fontSize];
	auto iter = charsWithSize.find(c);
	if (iter != charsWithSize.end())
		return (*iter).second;

	const GMGlyphInfo& glyph = createChar(c, fontSize, font);
	if (!glyph.valid)
	{
		//如果没有拿到当前的字形，需要换一种默认字体匹配
		return getCharInner(c, fontSize, candidate, candidate + 1);
	}
	return glyph;
}

GMFontHandle GMGlyphManager::addFontByFileName(const GMString& fontFileName)
{
	static GMString fontPath = GMPath::getSpecialFolderPath(GMPath::Fonts);
	return addFontByFullName(GMPath::fullname(fontPath, fontFileName));
}

GMFontHandle GMGlyphManager::addFontByFullName(const GMString& fontFullName)
{
	D(d);
	// 这是一个非线程安全的方法
	GMFont font;
	FT_Face face;
	FT_Error err = loadFace(fontFullName, &face);
	if (err == FT_Err_Ok)
	{
		font.fontPath = fontFullName;
		font.face = face;
		d->fonts.push_back(font);
		return d->fonts.size() - 1;
	}
	gm_error(L"warning: load font failed.");
	return GMInvalidFontHandle;
}

GMGlyphInfo& GMGlyphManager::insertChar(GMint fontSize, GMFontHandle font, GMwchar ch, const GMGlyphInfo& glyph)
{
	D(d);
	auto result = d->chars[font][fontSize].insert({ ch, glyph });
	GM_ASSERT(result.second);
	return (*(result.first)).second;
}

GMGlyphManager::GMGlyphManager(const IRenderContext* context)
{
	D(d);
	d->context = context;
	d->defaultFontSun = addFontByFileName("simhei.ttf");
	d->defaultFontTimesNewRoman = addFontByFileName("times.ttf");

	if (d->defaultFontSun == GMInvalidFontHandle)
		d->defaultFontSun = 0;
	if (d->defaultFontTimesNewRoman == GMInvalidFontHandle)
		d->defaultFontTimesNewRoman = 0;
}

GMGlyphManager::~GMGlyphManager()
{
	D(d);
	for (auto& font : d->fonts)
	{
		FT_Done_Face((FT_Face) font.face);
		font.face = nullptr;
	}
}

const GMGlyphInfo& GMGlyphManager::createChar(GMwchar c, GMFontSizePt fontSize, GMFontHandle font)
{
	D(d);
	static GMGlyphInfo errGlyph = { false };
	FT_Glyph glyph;
	FT_Error error;
	FT_UInt charIndex;
	GMFont* f = getFont(font);
	if (!f)
		return errGlyph;

	FT_Face face = (FT_Face) f->face;
	error = FT_Select_Charmap(face, FT_ENCODING_UNICODE);
	error = FT_Set_Char_Size(face, 0, fontSize << 6, GMScreen::dpi(), GMScreen::dpi());
	charIndex = FT_Get_Char_Index(face, c);
	if (charIndex == 0)
		return errGlyph;

	error = FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT);
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
			// 释放资源
			FT_Done_Glyph(glyph);
			return errGlyph;
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

	return insertChar(fontSize, font, c, glyphInfo);
}

GMFont* GMGlyphManager::getFont(GMFontHandle handle)
{
	D(d);
	if (handle >= d->fonts.size())
		return nullptr;
	return &d->fonts[handle];
}