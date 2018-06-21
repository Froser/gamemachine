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

void GMTypoStateMachine::createInstance(GMTypoEngine* engine, OUT GMTypoStateMachine** stateMachine)
{
	D(d);
	GM_ASSERT(stateMachine);
	*stateMachine = new GMTypoStateMachine(engine);
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
	const std::wstring& wstr = literature.toStdWString();
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
	if (!d->results.empty())
		eof.x = d->results[d->results.size() - 1].x;
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

void GMTypoEngine::setFont(GMFontHandle font)
{
	D(d);
	d->font = font;
}

void GMTypoEngine::createInstance(OUT ITypoEngine** engine)
{
	D(d);
	GM_ASSERT(engine);
	GMTypoStateMachine* stateMachine = nullptr;
	*engine = new GMTypoEngine(d->context);
}

GMint GMTypoEngine::getLineHeight()
{
	D(d);
	return d->lineHeight;
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

	if (d->options.newline && parseResult == GMTypoStateMachine::Newline)
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
		if (d->options.newline && result.x + result.width > d->options.typoArea.width)
		{
			newLine();
			result.x = d->current_x + glyph.bearingX;
			result.y = d->current_y;
		}
	}
	d->current_x += glyph.advance;
	result.advance = glyph.advance;

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

GMTypoTextBuffer::~GMTypoTextBuffer()
{
	D(d);
	GM_delete(d->engine);
}

void GMTypoTextBuffer::setBuffer(const GMString& buffer)
{
	D(d);
	d->buffer = buffer;
	setDirty();
}

void GMTypoTextBuffer::setSize(const GMRect& rc)
{
	D(d);
	d->rc = rc;
	d->rc.x = d->rc.y = 0;
	setDirty();
}

void GMTypoTextBuffer::setNewline(bool newline)
{
	D(d);
	if (d->newline != newline)
	{
		d->newline = newline;
		setDirty();
	}
}

void GMTypoTextBuffer::setChar(GMsize_t pos, GMwchar ch)
{
	D(d);
	d->buffer[pos] = ch;
	setDirty();
}

bool GMTypoTextBuffer::insertChar(GMsize_t pos, GMwchar ch)
{
	D(d);
	if (pos < 0)
		return false;

	if (pos == d->buffer.length())
	{
		d->buffer.append(ch);
	}
	else
	{
		GMString newStr = d->buffer.substr(0, pos);
		newStr.append(ch);
		newStr.append(d->buffer.substr(pos, d->buffer.length() - pos));
		d->buffer = std::move(newStr);
	}
	setDirty();
	return true;
}

bool GMTypoTextBuffer::removeChar(GMsize_t pos)
{
	D(d);
	if (pos < 0 || pos >= d->buffer.length())
		return false;

	GMString newStr = d->buffer.substr(0, pos);
	if (pos + 1 < d->buffer.length())
		newStr += d->buffer.substr(pos + 1, d->buffer.length() - pos - 1);
	d->buffer = std::move(newStr);
	setDirty();
	return true;
}

bool GMTypoTextBuffer::removeChars(GMsize_t startPos, GMsize_t endPos)
{
	D(d);
	if (startPos > endPos)
	{
		GM_SWAP(startPos, endPos);
	}

	if (startPos < 0 || startPos > d->buffer.length())
		return false;

	if (endPos < 0 || endPos > d->buffer.length())
		return false;

	if (startPos == endPos)
		return false;

	GMString newStrA = d->buffer.substr(0, startPos);
	if (endPos < d->buffer.length())
		newStrA.append(d->buffer.substr(endPos, d->buffer.length() - endPos));
	d->buffer = std::move(newStrA);
	setDirty();
	return true;
}

GMint GMTypoTextBuffer::getLength()
{
	D(d);
	GM_ASSERT((GMuint) std::numeric_limits<GMint>::max() > d->buffer.length());
	return d->buffer.length();
}

GMint GMTypoTextBuffer::getLineHeight()
{
	D(d);
	if (d->dirty)
		analyze();

	return d->engine->getLineHeight();
}

void GMTypoTextBuffer::analyze()
{
	D(d);
	GMTypoOptions options;
	options.typoArea = d->rc;
	options.newline = d->newline;
	d->engine->begin(d->buffer, options);
	d->dirty = false;
}

bool GMTypoTextBuffer::CPtoX(GMint cp, bool trail, GMint* x)
{
	D(d);
	if (d->dirty)
		analyze();

	if (!x)
		return false;
	
	if (cp < 0)
	{
		*x = 0;
		return true;
	}

	auto& r = d->engine->getResults();
	if (cp >= getLength())
		return CPtoX(cp - 1, true, x);

	if (trail)
		*x = r[cp].x + r[cp].advance;
	else
		*x = r[cp].x;
	return true;
}

bool GMTypoTextBuffer::XtoCP(GMint x, GMint* cp, bool* trail)
{
	D(d);
	if (d->dirty)
		analyze();

	if (x < 0)
		return 0;

	auto& r = d->engine->getResults();
	for (GMsize_t i = 0; i < r.size() - 1; ++i)
	{
		if (r[i].x == x)
		{
			if (cp)
				*cp = i - 1;

			if (trail)
				*trail = true;

			return true;
		}
		else if (r[i].x < x && r[i + 1].x > x)
		{
			if (cp)
				*cp = i;

			if (trail)
				*trail = false;

			return true;
		}
	}
	
	if (cp)
		*cp = r.size() - 1;
	if (trail)
		*trail = false;

	return true;
}

void GMTypoTextBuffer::getPriorItemPos(GMint cp, GMint* prior)
{
	*prior = cp;
}

void GMTypoTextBuffer::getNextItemPos(GMint cp, GMint* next)
{
	*next = cp;
}