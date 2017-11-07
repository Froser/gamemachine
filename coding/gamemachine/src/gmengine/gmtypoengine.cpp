#include "stdafx.h"
#include "gmtypoengine.h"
#include "foundation/gamemachine.h"
#include <regex>

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

GMTypoStateMachine::ParseResult GMTypoStateMachine::parse(REF GMWchar& ch)
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
		GMint sz;
		std::string str = value.toStdString();
		SAFE_SSCANF(str.c_str(), "%d", &sz);
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
	setFontSize(d->options.defaultFontSize);
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
}

bool GMTypoEngine::isValidTypeFrame()
{
	D(d);
	return !(d->options.typoArea.width < 0 || d->options.typoArea.height < 0);
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