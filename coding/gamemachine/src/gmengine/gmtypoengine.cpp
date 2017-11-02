#include "stdafx.h"
#include "gmtypoengine.h"
#include "foundation/gamemachine.h"

GMTypoIterator::GMTypoIterator(ITypoEngine* typo, GMint index)
{
	D(d);
	d->typo = typo;
	d->index = index;

	if (!index)
	{
		d->result = d->typo->getTypoResult(index);
		d->invalid = false;
	}
}

const GMTypoResult& GMTypoIterator::operator*()
{
	D(d);
	GM_ASSERT(!d->invalid);
	return d->result;
}

GMTypoIterator& GMTypoIterator::operator ++(int)
{
	D(d);
	GM_ASSERT(!d->invalid);
	d->result = d->typo->getTypoResult(++d->index);
	return *this;
}

bool GMTypoIterator::operator==(const GMTypoIterator& rhs)
{
	D(d);
	D_OF(d_rhs, &rhs);
	return d->index == d_rhs->index;
}

bool GMTypoIterator::operator!=(const GMTypoIterator& rhs)
{
	return !(*this == rhs);
}

GMTypoIterator GMTypoEngine::begin(const GMString& literature, const GMTypoOptions& options)
{
	D(d);
	d->options = options;
	d->fontSize = d->options.defaultFontSize;
	d->literature = literature.toStdWString();

	// 获取行高
	GMGlyphManager* glyphManager = GM.getGlyphManager();
	std::wstring wstr = literature.toStdWString();
	const GMWchar* p = wstr.c_str();
	while (*p)
	{
		const GlyphInfo& glyph = glyphManager->getChar(*p, d->fontSize);
		if (d->lineHeight < glyph.height)
			d->lineHeight = glyph.height;
		++p;
	}

	return GMTypoIterator(this, 0);
}

GMTypoIterator GMTypoEngine::end()
{
	D(d);
	return GMTypoIterator(this, d->literature.length());
}

GMTypoResult GMTypoEngine::getTypoResult(GMint index)
{
	D(d);
	GMTypoResult result;
	//TODO COLOR
	result.color[0] = 0;
	result.color[1] = 1;
	result.color[2] = 0;

	const GlyphInfo& glyph = d->glyphManager->getChar(d->literature[index], d->fontSize);
	result.glyph = &glyph;

	result.lineHeight = d->lineHeight;
	result.fontSize = d->fontSize;

	result.x = d->current_x + glyph.bearingX;
	result.y = 0;
	result.width = glyph.width;
	result.height = glyph.height;
	d->current_x += glyph.advance;

	return result;
}
