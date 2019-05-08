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

	FT_Error loadFace(const GMString& fontFullPath, FT_Face* face)
	{
		FT_Error err = FT_New_Face(g_lib.library, fontFullPath.toStdString().c_str(), 0, face);
		return err;
	}

	FT_Error loadFace(const GMBuffer& buffer, FT_Face* face)
	{
		FT_Error err = FT_New_Memory_Face(g_lib.library, buffer.getData(), (FT_Long) buffer.getSize(), 0, face);
		return err;
	}
}

const GMGlyphInfo& GMGlyphManager::getChar(GMwchar c, GMint32 fontSize, GMFontHandle font)
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
	for (GMsize_t i = 0; i < d->fonts.size(); ++i)
	{
		if (d->fonts[i].fontPath == fontFullName)
			return gm_sizet_to_uint(i);
	}

	GMFont font;
	FT_Face face;
	FT_Error err = loadFace(fontFullName, &face);
	if (err == FT_Err_Ok)
	{
		font.fontPath = fontFullName;
		font.face = face;
		d->fonts.push_back(font);
		return gm_sizet_to_uint(d->fonts.size() - 1);
	}
	gm_error(gm_dbg_wrap("load font failed."));
	return GMInvalidFontHandle;
}

GMFontHandle GMGlyphManager::addFontByMemory(GMBuffer&& buffer)
{
	D(d);
	GMFont font;
	FT_Face face;
	FT_Error err = loadFace(buffer, &face);

	// FT规定在FT_Done_Face之前不能释放Buffer，因此这里所有的操作必须使用移动语义，防止Buffer释放
	if (err == FT_Err_Ok)
	{
		font.buffer = std::move(buffer);
		font.face = face;
		d->fonts.push_back(std::move(font));
		return gm_sizet_to_uint(d->fonts.size() - 1);
	}
	gm_error(gm_dbg_wrap("load font failed."));
	return GMInvalidFontHandle;
}

GMGlyphInfo& GMGlyphManager::insertChar(GMint32 fontSize, GMFontHandle font, GMwchar ch, const GMGlyphInfo& glyph)
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
	GM_ASSERT(error == FT_Err_Ok);

	error = FT_Set_Char_Size(face, 0, fontSize << 6, GMScreen::horizontalResolutionDpi(), GMScreen::verticalResolutionDpi());
	GM_ASSERT(error == FT_Err_Ok);

	charIndex = FT_Get_Char_Index(face, c);
	if (charIndex == 0)
		return errGlyph;

	error = FT_Load_Glyph(face, charIndex, FT_LOAD_DEFAULT);
	GM_ASSERT(error == FT_Err_Ok);

	error = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
	GM_ASSERT(error == FT_Err_Ok);

	error = FT_Get_Glyph(face->glyph, &glyph);
	GM_ASSERT(error == FT_Err_Ok);

	error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
	GM_ASSERT(error == FT_Err_Ok);
	FT_BitmapGlyph bitmapGlyph = (FT_BitmapGlyph)glyph;

	// 创建结构
	if (d->cursor_u + bitmapGlyph->bitmap.width > CANVAS_WIDTH)
	{
		d->cursor_v += d->maxHeight + 1;
		d->maxHeight = 0;
		d->cursor_u = 0;
		if (d->cursor_v + bitmapGlyph->bitmap.rows > CANVAS_HEIGHT)
		{
			gm_error(gm_dbg_wrap("no texture space for glyph!"));
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