#include "stdafx.h"
#include "typopainter.h"
#include "ft2build.h"
#include "freetype/freetype.h"

TypoPainter::TypoPainter()
{
	FT_Library library;
	FT_Init_FreeType(&library);
}