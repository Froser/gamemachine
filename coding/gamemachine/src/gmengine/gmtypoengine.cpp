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

GMTypoIterator& GMTypoIterator::operator ++()
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

//////////////////////////////////////////////////////////////////////////
GMTypoStateMachine::GMTypoStateMachine(GMTypoEngine* engine)
{
	D(d);
	GM_ASSERT(engine);
	d->typoEngine = engine;
}

GMTypoStateMachine::ParseState GMTypoStateMachine::parse(GMWchar ch, REF GMTypoResult& result)
{
	//TODO
	setColor(result, 0, 1, 0);
	return Okay;
}

void GMTypoStateMachine::setColor(REF GMTypoResult& result, GMfloat r, GMfloat g, GMfloat b)
{
	result.color[0] = r;
	result.color[1] = g;
	result.color[2] = b;
}

//////////////////////////////////////////////////////////////////////////
GMTypoEngine::GMTypoEngine()
{
	D(d);
	d->stateMachine = new GMTypoStateMachine(this);
}

GMTypoEngine::GMTypoEngine(AUTORELEASE GMTypoStateMachine* stateMachine)
{
	D(d);
	d->stateMachine = stateMachine;
}

GMTypoEngine::~GMTypoEngine()
{
	D(d);
	GM_delete(d->stateMachine);
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
		const GMGlyphInfo& glyph = glyphManager->getChar(*p, d->fontSize);
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
	GMWchar ch = d->literature[index];
	GMTypoStateMachine::ParseState state = d->stateMachine->parse(ch, result);
	if (state == GMTypoStateMachine::Ignore)
	{
		result.valid = false;
		return result;
	}

	const GMGlyphInfo& glyph = d->glyphManager->getChar(ch, d->fontSize);
	result.glyph = &glyph;

	result.lineHeight = d->lineHeight;
	result.fontSize = d->fontSize;

	result.x = d->current_x + glyph.bearingX;
	result.y = d->current_y;
	result.width = glyph.width;
	result.height = glyph.height;
	if (isValidTypeFrame())
	{
		// 如果给定了一个合法的绘制区域，且出现超出绘制区域的情况
		if (result.x + result.width > d->options.typoArea.width)
		{
			d->current_x = 0;
			d->current_y += d->lineHeight + d->options.lineSpacing;
			result.x = d->current_x + glyph.bearingX;
			result.y = d->current_y;
		}
	}
	d->current_x += glyph.advance;

	return result;
}

bool GMTypoEngine::isValidTypeFrame()
{
	D(d);
	return !(d->options.typoArea.width < 0 || d->options.typoArea.height < 0);
}