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

GMTypoResult GMTypoIterator::operator*()
{
	D(d);
	GMTypoResult r = d->typo->getResults().results[d->index];
	r.x -= d->offset[0];
	r.y -= d->offset[1];
	return r;
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

void GMTypoIterator::setOffset(GMsize_t cp)
{
	D(d);
	decltype(auto) result = d->typo->getResults().results[cp];
	d->offset[0] = result.x;
	d->offset[1] = result.y;
}

//////////////////////////////////////////////////////////////////////////
GMTypoStateMachine::GMTypoStateMachine(GMTypoEngine* engine)
{
	D(d);
	GM_ASSERT(engine);
	d->typoEngine = engine;
}

GMTypoStateMachine::ParseResult GMTypoStateMachine::parse(const GMTypoOptions& options, REF GMwchar& ch)
{
	D(d);
	bool isPlainText = options.plainText;
	bool newline = options.newline;

	if (ch == '\r')
		return Ignore;

	switch (d->state)
	{
	case GMTypoStateMachineParseState::Literature:
	{
		if (!isPlainText && ch == '[')
		{
			d->state = GMTypoStateMachineParseState::WaitingForCommand;
			return Ignore;
		}

		if (ch == '\n' && newline)
			return Newline;

		return Okay;
	}
	case GMTypoStateMachineParseState::WaitingForCommand:
	{
		if (!isPlainText && ch == '[')
		{
			d->state = GMTypoStateMachineParseState::Literature;
			return Okay;
		}
		else if (!isPlainText && ch == ']')
		{
			d->state = GMTypoStateMachineParseState::Literature;
			return Ignore;
		}
		else
		{
			d->state = GMTypoStateMachineParseState::ParsingSymbol;
			d->parsedSymbol = "";
			return parse(options, ch);
		}
		break;
	}
	case GMTypoStateMachineParseState::ParsingSymbol:
	{
		if (!isPlainText && ch == ']')
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

GMTypoIterator GMTypoEngine::begin(const GMString& literature, const GMTypoOptions& options, GMsize_t start)
{
	D(d);
	d->literature = literature.toStdWString();

	// 如果使用缓存且非空，直接返回
	// 否则还是会尝试解析一下
	if (options.useCache && !d->results.empty())
	{
		d->options.useCache = options.useCache;
		d->options.renderStart = options.renderStart;
		d->options.renderEnd = options.renderEnd;

		// 如果没有指定renderEnd，默认为0，很可能是调用者忘记设置了，在此处ASSERT一下
		GM_ASSERT(literature.isEmpty() || (literature.length() > 0 && options.renderEnd > 0));

		// 在使用缓存绘制时，我们一般通过renderStart和renderEnd指定了一块区域，因此绘制出来后需要做一个偏移
		GMTypoIterator iter = GMTypoIterator(this, options.renderStart);
		iter.setOffset(options.renderStart);
		return std::move(iter);
	}

	d->options = options;
	if (start == 0)
	{
		d->results.clear();
		d->current_x = d->current_y = 0;
		d->currentLineNo = 1;
	}
	else
	{
		// 从某一个点开始局部排版
		// 我们不知道此时的排版结果，所以我们只能拿上一个字符的结果，并且从上一个字符开始排版
		decltype(auto) last = &d->results[--start];
		if (start == 0)
		{
			d->results.clear();
			d->current_x = d->current_y = 0;
			d->currentLineNo = 1;
		}
		else
		{
			last = &d->results[start];
			d->current_x = last->x - last->bearingX; // 横向排版，计算x坐标的时候，current_x加上了bearingX，所以要减去它，见getTypoResult()
			d->current_y = last->y;
			d->currentLineNo = last->lineNo;
			d->results.erase(d->results.begin() + start, d->results.end());
		}
	}

	setFontSize(d->options.defaultFontSize);

	const std::wstring& wstr = literature.toStdWString();

	// lineHeight为0表示自动获取行高，获取第1个字符的高度
	if (d->lineHeight == 0)
	{
		GMGlyphManager* glyphManager = d->context->getEngine()->getGlyphManager();
		const GMwchar* p = wstr.c_str();
		while (*p)
		{
			const GMGlyphInfo& glyph = glyphManager->getChar(*p, d->fontSize, d->font);
			d->lineHeight = glyph.height;
			break;
		}
	}

	auto _adjustNewLineSepResult = [=](auto& target, bool copyLineNo) {
		if (!d->results.empty())
		{
			decltype(auto) last = d->results[d->results.size() - 1];
			// 换行符没有size，跟随它的前一个字符，但是如果前一个字符也是换行符，那么它的y坐标将新起一行
			if (!last.newLineOrEOFSeparator)
			{
				target.y = last.y;
				target.x = last.x + last.advance;
				if (copyLineNo)
					target.lineNo = last.lineNo;
			}
		}
	};

	GMint rows = 0;
	GMsize_t len = wstr.length();
	for (GMsize_t i = start; i < len; ++i)
	{
		GMTypoResult result = getTypoResult(i);
		if (!result.valid)
			continue;

		if (result.newLineOrEOFSeparator)
			_adjustNewLineSepResult(result, false);
		d->results.push_back(result);
	}

	GMTypoResult eof;
	eof.lineNo = d->currentLineNo;
	eof.newLineOrEOFSeparator = true;
	_adjustNewLineSepResult(eof, true);
	d->results.push_back(eof);

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
			pCurrent->y += (d->options.typoArea.height - eof.lineNo * d->lineHeight) / 2;

			pCurrent++;
		}
	}

	return GMTypoIterator(this, 0);
}

GMTypoIterator GMTypoEngine::end()
{
	D(d);
	if (d->options.useCache)
		return GMTypoIterator(this, d->options.renderEnd);

	return GMTypoIterator(this, d->results.size() - 1);
}

void GMTypoEngine::setFont(GMFontHandle font)
{
	D(d);
	d->font = font;
}

void GMTypoEngine::setLineHeight(GMint lineHeight)
{
	D(d);
	d->lineHeight = lineHeight;
}

void GMTypoEngine::createInstance(OUT ITypoEngine** engine)
{
	D(d);
	GM_ASSERT(engine);
	GMTypoStateMachine* stateMachine = nullptr;
	*engine = new GMTypoEngine(d->context);
}

GMTypoResult GMTypoEngine::getTypoResult(GMsize_t index)
{
	D(d);
	GMTypoResult result;
	result.lineNo = d->currentLineNo;

	GMwchar ch = d->literature[index];
	GMTypoStateMachine::ParseResult parseResult = d->stateMachine->parse(d->options, ch);

	if (parseResult == GMTypoStateMachine::Ignore)
	{
		result.valid = false;
		return result;
	}

	if (d->options.newline && parseResult == GMTypoStateMachine::Newline)
	{
		result.lineNo = d->currentLineNo;
		result.newLineOrEOFSeparator = true;
		result.y = d->current_y;
		result.height = d->lineHeight;
		newLine();
		return result;
	}

	const GMGlyphInfo& glyph = d->glyphManager->getChar(ch, d->fontSize, d->font);
	result.glyph = &glyph;

	if (std::isspace(ch, std::locale()))
		result.isSpace = true;

	result.fontSize = d->fontSize;
	result.x = d->current_x + glyph.bearingX;
	result.y = d->current_y;
	result.width = glyph.width;
	result.height = glyph.height;
	result.advance = glyph.advance;
	result.bearingX = glyph.bearingX;
	result.bearingY = glyph.bearingY;
	if (isValidTypeFrame())
	{
		// 如果给定了一个合法的绘制区域，且出现超出绘制区域的情况
		if (d->options.newline && result.x + result.advance > d->options.typoArea.width)
		{
			newLine();
			result.x = d->current_x + glyph.bearingX;
			result.y = d->current_y;
			result.lineNo = d->currentLineNo;
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
	++d->currentLineNo;
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

GMTypoResultInfo GMTypoEngine::getResults()
{
	D(d);
	GMTypoResultInfo r = {
		d->results,
		static_cast<GMint>(d->lineHeight),
		d->options.lineSpacing
	};
	return r;
}

const GMTypoResultInfo GMTypoEngine::getResults() const
{
	return const_cast<GMTypoEngine*>(this)->getResults();
}

GM_PRIVATE_OBJECT(GMTypoTextTransactionAtom)
{
	GMsize_t cp = 0;
	GMString addedContent;
	GMString removedContent;
	GMTypoTextBuffer* buffer = nullptr;
};

class GMTypoTextTransactionAtom : public ITransactionAtom
{
	GM_DECLARE_PRIVATE(GMTypoTextTransactionAtom)

public:
	GMTypoTextTransactionAtom(
		GMTypoTextBuffer* buffer,
		GMsize_t cp,
		GMString addedContent,
		GMString removedContent
	);

	virtual void execute() override;
	virtual void unexecute() override;
};

GMTypoTextTransactionAtom::GMTypoTextTransactionAtom(
	GMTypoTextBuffer* buffer,
	GMsize_t cp,
	GMString addedContent,
	GMString removedContent
)
{
	D(d);
	d->buffer = buffer;
	d->cp = cp;
	d->addedContent = std::move(addedContent);
	d->removedContent = std::move(removedContent);
}

void GMTypoTextTransactionAtom::execute()
{
	D(d);
	d->buffer->removeChars(d->cp, d->cp + d->removedContent.length());
	d->buffer->insertString(d->cp, d->addedContent);
}

void GMTypoTextTransactionAtom::unexecute()
{
	D(d);
	d->buffer->removeChars(d->cp, d->cp + d->addedContent.length());
	d->buffer->insertString(d->cp, d->removedContent);
}

GMTypoTextBuffer::~GMTypoTextBuffer()
{
	D(d);
	GM_delete(d->engine);
}

void GMTypoTextBuffer::setBuffer(const GMString& buffer)
{
	D(d);
	d->buffer = buffer.replace(L"\r", L""); //去掉\r
	markDirty();
}

void GMTypoTextBuffer::setSize(const GMRect& rc)
{
	D(d);
	d->rc = rc;
	d->rc.x = d->rc.y = 0;
	markDirty();
}

void GMTypoTextBuffer::setChar(GMsize_t pos, GMwchar ch)
{
	D(d);
	if (ch == '\r')
		return;

	d->transactionMgr.addAtom(new GMTypoTextTransactionAtom(this, pos, ch, d->buffer[pos]));
	d->buffer[pos] = ch;
	markDirty();
}

bool GMTypoTextBuffer::insertChar(GMsize_t pos, GMwchar ch)
{
	D(d);
	if (ch == '\r')
		return false;

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

	d->transactionMgr.addAtom(new GMTypoTextTransactionAtom(this, pos, ch, L""));
	markDirty();
	return true;
}

bool GMTypoTextBuffer::insertString(GMsize_t pos, const GMString& str)
{
	D(d);
	if (pos < 0)
		return false;

	if (str.isEmpty())
		return true;

	if (pos == d->buffer.length())
	{
		d->buffer.append(str.replace(L"\r", L""));
	}
	else
	{
		GMString newStr = d->buffer.substr(0, pos);
		newStr.append(str.replace(L"\r", L""));
		newStr.append(d->buffer.substr(pos, d->buffer.length() - pos));
		d->buffer = std::move(newStr);
	}

	d->transactionMgr.addAtom(new GMTypoTextTransactionAtom(this, pos, str, L""));
	markDirty();
	return true;
}

bool GMTypoTextBuffer::removeChar(GMsize_t pos)
{
	D(d);
	if (pos < 0 || pos >= d->buffer.length())
		return false;

	GMwchar removedChar = d->buffer[pos];
	GMString newStr = d->buffer.substr(0, pos);
	if (pos + 1 < d->buffer.length())
		newStr += d->buffer.substr(pos + 1, d->buffer.length() - pos - 1);
	d->buffer = std::move(newStr);

	d->transactionMgr.addAtom(new GMTypoTextTransactionAtom(this, pos, L"", removedChar));
	markDirty();
	return true;
}

bool GMTypoTextBuffer::removeChars(GMsize_t startPos, GMsize_t endPos)
{
	D(d);
	if (endPos == startPos)
		return true;

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

	GMString removedChars = d->buffer.substr(startPos, endPos - startPos);
	GMString newStrA = d->buffer.substr(0, startPos);
	if (endPos < d->buffer.length())
		newStrA.append(d->buffer.substr(endPos, d->buffer.length() - endPos));
	d->buffer = std::move(newStrA);
	d->transactionMgr.addAtom(new GMTypoTextTransactionAtom(this, startPos, L"", removedChars));
	markDirty();
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
		analyze(0);

	return d->engine->getResults().lineHeight;
}

bool GMTypoTextBuffer::isPlainText() GM_NOEXCEPT
{
	return true;
}

GMwchar GMTypoTextBuffer::getChar(GMsize_t pos)
{
	D(d);
	return d->buffer[pos];
}

void GMTypoTextBuffer::analyze(GMint start)
{
	D(d);
	GMTypoOptions options;
	options.typoArea = d->rc;
	options.newline = false;
	options.plainText = isPlainText();
	d->engine->begin(d->buffer, options, start);
	d->dirty = false;
}

bool GMTypoTextBuffer::CPtoX(GMint cp, bool trail, GMint* x)
{
	D(d);
	if (!x)
		return false;
	
	if (cp < 0)
	{
		*x = 0;
		return true;
	}

	if (d->dirty)
		analyze(cp);

	auto& r = d->engine->getResults().results;
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
	if (x < 0)
	{
		if (cp)
			*cp = 0;

		if (trail)
			*trail = false;
		return false;
	}

	if (d->dirty)
		analyze(0);

	auto& r = d->engine->getResults().results;
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
	D(d);
	auto& r = d->engine->getResults().results;
	for (GMint i = cp - 1; i > 0; --i)
	{
		if (!r[i].isSpace && r[i - 1].isSpace)
		{
			*prior = i;
			return;
		}
	}

	*prior = 0;
}

void GMTypoTextBuffer::getNextItemPos(GMint cp, GMint* next)
{
	D(d);
	auto& r = d->engine->getResults().results;
	for (GMint i = cp; i < static_cast<GMint>(r.size() - 2); ++i)
	{
		if (r[i].isSpace && !r[i + 1].isSpace)
		{
			*next = i + 1;
			return;
		}
	}

	*next = r.size() - 1;
}