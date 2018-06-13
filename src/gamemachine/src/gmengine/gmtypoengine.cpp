#include "stdafx.h"
#include "gmtypoengine.h"
#include "foundation/gamemachine.h"
#include <regex>

GMTypoIterator::GMTypoIterator(ITypoEngine* typo, GMsize_t index)
{
	D(d);
	d->typo = typo;
	d->index = index;
}

const GMTypoResult& GMTypoIterator::operator*()
{
	D(d);
	return d->typo->getResults()[d->index];
}

GMTypoIterator& GMTypoIterator::operator ++()
{
	D(d);
	++d->index;
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

GMTypoStateMachine::ParseResult GMTypoStateMachine::parse(REF GMwchar& ch)
{
	D(d);
	switch (d->state)
	{
	case GMTypoStateMachineParseState::Literature:
	{
		if (ch == '[')
		{
			d->state = GMTypoStateMachineParseState::WaitingForCommand;
			return Ignore;
		}
		return Okay;
	}
	case GMTypoStateMachineParseState::WaitingForCommand:
	{
		if (ch == '[')
		{
			d->state = GMTypoStateMachineParseState::Literature;
			return Okay;
		}
		else if (ch == ']')
		{
			d->state = GMTypoStateMachineParseState::Literature;
			return Ignore;
		}
		else
		{
			d->state = GMTypoStateMachineParseState::ParsingSymbol;
			d->parsedSymbol = "";
			return parse(ch);
		}
		break;
	}
	case GMTypoStateMachineParseState::ParsingSymbol:
	{
		if (ch == ']')
		{
			d->state = GMTypoStateMachineParseState::Literature;
			return applyAttribute();
		}
		else
		{
			d->parsedSymbol += ch;
			return Ignore;
		}
	}
	default:
		GM_ASSERT(false);
		break;
	}
	return Okay;
}

GMTypoStateMachine::ParseResult GMTypoStateMachine::applyAttribute()
{
	D(d);
	GMString value;
	if (parsePair("color", value))
	{
		GMfloat rgb[3];
		bool b = GMConvertion::hexToRGB(value, rgb);
		GM_ASSERT(b);
		setColor(rgb);
		return Ignore;
	}
	else if (parsePair("size", value))
	{
		GMint sz = GMString::parseInt(value);
		setFontSize(sz);
		return Ignore;
	}
	else if (preciseParse("n"))
	{
		return Newline;
	}

	GM_ASSERT(false);
	return Okay;
}

void GMTypoStateMachine::setColor(GMfloat rgb[3])
{
	D(d);
	d->typoEngine->setColor(rgb);
}

void GMTypoStateMachine::setFontSize(GMint sz)
{
	D(d);
	d->typoEngine->setFontSize(sz);
}

bool GMTypoStateMachine::parsePair(const GMString& key, REF GMString& value)
{
	D(d);
	GMString expr = ("(") + key + ")\\s*=\\s*(.*)";
	std::regex regPair(expr.toStdString());
	std::smatch match;
	std::string symbol = d->parsedSymbol.toStdString();
	if (std::regex_search(symbol, match, regPair))
	{
		GM_ASSERT(match.size() >= 3);
		value = match[2].str();
		return true;
	}
	return false;
}

bool GMTypoStateMachine::preciseParse(const GMString& name)
{
	D(d);
	return d->parsedSymbol == name;
}

//////////////////////////////////////////////////////////////////////////
GMTypoEngine::GMTypoEngine(const IRenderContext* context)
{
	D(d);
	d->stateMachine = new GMTypoStateMachine(this);
	d->context = context;
	d->glyphManager = d->context->getEngine()->getGlyphManager();
	d->font = d->glyphManager->getTimesNewRoman();
}

GMTypoEngine::GMTypoEngine(const IRenderContext* context, AUTORELEASE GMTypoStateMachine* stateMachine)
{
	D(d);
	d->stateMachine = stateMachine;
	d->context = context;
	d->glyphManager = d->context->getEngine()->getGlyphManager();
	d->font = d->glyphManager->getTimesNewRoman();
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
	d->current_x = d->current_y = 0;
	d->results.clear();
	setFontSize(d->options.defaultFontSize);
	d->literature = literature.toStdWString();

	// 获取行高
	GMGlyphManager* glyphManager = d->context->getEngine()->getGlyphManager();
	std::wstring wstr = literature.toStdWString();
	const GMwchar* p = wstr.c_str();
	while (*p)
	{
		const GMGlyphInfo& glyph = glyphManager->getChar(*p, d->fontSize, d->font);
		if (d->lineHeight < glyph.height)
			d->lineHeight = glyph.height;
		++p;
	}

	GMint rows = 0;
	GMsize_t len = wstr.length();
	for (GMsize_t i = 0; i < len; ++i)
	{
		GMTypoResult result = getTypoResult(i);
		if (result.newLineOrEOFSeparator)
			++rows;
		d->results.push_back(result);
	}
	GMTypoResult eof;
	eof.newLineOrEOFSeparator = true;
	d->results.push_back(eof);
	++rows;

	GMsize_t sz = d->results.size();
	if (d->options.center && sz > 0)
	{
		GMTypoResult* pStart = const_cast<GMTypoResult*>(d->results.data());
		GMTypoResult* pCurrent = pStart;
		const GMTypoResult* pLast = &d->results[sz - 1];

		while (pCurrent != pLast + 1)
		{
			if (pCurrent->newLineOrEOFSeparator)
			{
				// 如果是一个行分隔符，计算出本行的宽度
				GMTypoResult* prev = pCurrent - 1;
				GMint width = prev->x + prev->width - pStart->x;
				while (pStart != pCurrent)
				{
					pStart->x += (d->options.typoArea.width - width) / 2;
					++pStart;
				}
			}

			//调整高度
			pCurrent->y += (d->options.typoArea.height - rows * d->lineHeight) / 2;

			pCurrent++;
		}
	}

	return GMTypoIterator(this, 0);
}

GMTypoIterator GMTypoEngine::end()
{
	D(d);
	return GMTypoIterator(this, d->literature.length());
}

GMTypoResult GMTypoEngine::getTypoResult(GMsize_t index)
{
	D(d);
	GMTypoResult result;
	GMwchar ch = d->literature[index];
	GMTypoStateMachine::ParseResult parseResult = d->stateMachine->parse(ch);
	if (parseResult == GMTypoStateMachine::Ignore)
	{
		result.valid = false;
		return result;
	}

	if (parseResult == GMTypoStateMachine::Newline)
	{
		newLine();
		result.valid = false;
		return result;
	}

	const GMGlyphInfo& glyph = d->glyphManager->getChar(ch, d->fontSize, d->font);
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
			newLine();
			result.x = d->current_x + glyph.bearingX;
			result.y = d->current_y;
		}
	}
	d->current_x += glyph.advance;

	// 拷贝状态
	result.color[0] = d->color[0];
	result.color[1] = d->color[1];
	result.color[2] = d->color[2];
	result.color[3] = 1.f;

	return result;
}

void GMTypoEngine::newLine()
{
	D(d);
	d->current_x = 0;
	d->current_y += d->lineHeight + d->options.lineSpacing;

	GMTypoResult newLineSep;
	newLineSep.valid = false;
	newLineSep.newLineOrEOFSeparator = true;
	d->results.push_back(newLineSep);
}

bool GMTypoEngine::isValidTypeFrame()
{
	D(d);
	return !(d->options.typoArea.width < 0 || d->options.typoArea.height < 0);
}

void GMTypoEngine::setFont(GMFontHandle font)
{
	// 设置首选字体
	D(d);
	d->font = font;
}

void GMTypoEngine::setColor(GMfloat rgb[3])
{
	D(d);
	d->color[0] = rgb[0];
	d->color[1] = rgb[1];
	d->color[2] = rgb[2];
}

void GMTypoEngine::setFontSize(GMint pt)
{
	D(d);
	d->fontSize = pt;
}

const Vector<GMTypoResult>& GMTypoEngine::getResults()
{
	D(d);
	return d->results;
}