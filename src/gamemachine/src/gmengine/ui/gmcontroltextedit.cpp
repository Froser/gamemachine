#include "stdafx.h"
#include <gmtypoengine.h>
#include "gmcontroltextedit.h"
#include "foundation/gamemachine.h"

GM_DEFINE_SIGNAL(GMControlTextEdit::textChanged);

BEGIN_NS

GM_PRIVATE_OBJECT(GMMultiLineTypoTextBuffer)
{
	GMint lineSpacing = 5;
};

class GMMultiLineTypoTextBuffer : public GMTypoTextBuffer
{
	GM_DECLARE_PRIVATE_AND_BASE(GMMultiLineTypoTextBuffer, GMTypoTextBuffer)

public:
	GMMultiLineTypoTextBuffer() = default;

public:
	virtual void analyze() override;
	virtual bool CPtoX(GMint cp, bool trail, GMint* x) { return false; }
	virtual bool XtoCP(GMint x, GMint* cp, bool* trail) { return false; }
	virtual bool CPtoXY(GMint cp, bool trail, GMint* x, GMint* y);
	virtual bool XYtoCP(GMint x, GMint y, GMint* cp);
	
public:
	GMint CPToLineNumber(GMint cp);
	GMint findFirstCPInOneLine(GMint cp);
	GMint findLastCPInOneLine(GMint cp);

public:
	inline void setLineSpacing(GMint lineSpacing) GM_NOEXCEPT
	{
		D(d);
		d->lineSpacing = lineSpacing;
	}

	inline GMint getLineSpacing() GM_NOEXCEPT
	{
		D(d);
		return d->lineSpacing;
	}
};

void GMMultiLineTypoTextBuffer::analyze()
{
	D(d);
	D_BASE(db, Base);
	GMTypoOptions options;
	options.typoArea = db->rc;
	options.newline = true;
	options.plainText = true;
	options.lineSpacing = d->lineSpacing;
	db->engine->begin(db->buffer, options);
	db->dirty = false;
}

bool GMMultiLineTypoTextBuffer::CPtoXY(GMint cp, bool trail, GMint* x, GMint* y)
{
	D_BASE(d, Base);
	if (!x || !y)
		return false;

	if (cp < 0)
	{
		*x = 0;
		*y = 0;
		return true;
	}

	if (d->dirty)
		analyze();

	decltype(auto) r = d->engine->getResults();
	if (cp >= getLength())
		return CPtoXY(cp - 1, true, x, y);

	if (trail)
		*x = r[cp].x + r[cp].advance;
	else
		*x = r[cp].x;

	*y = r[cp].y;
	return true;
}

bool GMMultiLineTypoTextBuffer::XYtoCP(GMint x, GMint y, GMint* cp)
{
	D_BASE(d, Base);
	if (x < 0 || y < 0)
		return false;

	if (!cp)
		return false;

	if (d->dirty)
		analyze();

	decltype(auto) r = d->engine->getResults();
	if (r.empty())
	{
		*cp = 0;
		return true;
	}

	// r表示排版结果，最后为一个eof符号
	const GMPoint pt = { x, y };
	for (GMsize_t i = 0; i < r.size() - 1; ++i)
	{
		GMRect glyphRc = {
			static_cast<GMint>(r[i].x),
			static_cast<GMint>(r[i].y),
			static_cast<GMint>(r[i].advance),
			static_cast<GMint>(r[i].lineHeight)
		};

		if (GM_inRect(glyphRc, pt))
		{
			*cp = i;
			return true;
		}
	}

	// 如果没有选中某个字符，那么选择这一行末尾
	constexpr GMint s_range = 100000; //一个足够大的值，表示一行的范围
	GMint currentLine = 0;
	GMint rowX, rowY;
	for (GMsize_t i = 0; i < r.size() - 1; ++i)
	{
		if (currentLine == r[i].lineNo)
			continue;

		currentLine = r[i].lineNo;
		CPtoXY(i, true, &rowX, &rowY);

		GMRect rowRc = {
			static_cast<GMint>(-s_range),
			static_cast<GMint>(r[i].y),
			static_cast<GMint>(s_range * 2),
			static_cast<GMint>(r[i].lineHeight)
		};
		if (GM_inRect(rowRc, pt))
		{
			*cp = findLastCPInOneLine(i);
			return true;
		}
	}

	// 还没有的话，选中最后的CP
	if (cp)
		*cp = r.size() - 1;

	return true;
}

GMint GMMultiLineTypoTextBuffer::CPToLineNumber(GMint cp)
{
	D_BASE(d, Base);
	if (cp < 0)
		return 1;

	if (d->dirty)
		analyze();

	decltype(auto) r = d->engine->getResults();
	if (cp > static_cast<GMint>(r.size()))
		return r[r.size() - 1].lineNo;

	return r[cp].lineNo;
}

GMint GMMultiLineTypoTextBuffer::findFirstCPInOneLine(GMint cp)
{
	D_BASE(d, Base);
	if (d->dirty)
		analyze();

	decltype(auto) r = d->engine->getResults();
	if (cp >= static_cast<GMint>(r.size()))
		cp = static_cast<GMint>(r.size() - 1);

	auto lineNo = r[cp].lineNo;
	for (auto i = static_cast<GMint>(cp); i >= 0; --i)
	{
		if (r[i].lineNo < lineNo)
			return i + 1;
	}
	return 0;
}

GMint GMMultiLineTypoTextBuffer::findLastCPInOneLine(GMint cp)
{
	D_BASE(d, Base);
	if (d->dirty)
		analyze();

	decltype(auto) r = d->engine->getResults();
	if (cp >= static_cast<GMint>(r.size()))
		cp = static_cast<GMint>(r.size() - 1);

	auto lineNo = r[cp].lineNo;
	for (GMsize_t i = cp; i < r.size(); ++i)
	{
		if (r[i].lineNo > lineNo)
			return i - 1;
	}
	return r.size() - 1;
}
END_NS
//////////////////////////////////////////////////////////////////////////

class GMControlTextEditBorder : public GMControlBorder
{
public:
	GMControlTextEditBorder(GMWidget* widget) : GMControlBorder(widget) { initStyles(widget); }

private:
	void initStyles(GMWidget* widget);
};

GMControlTextEdit::GMControlTextEdit(GMWidget* widget)
	: GMControl(widget)
{
	D(d);
	d->borderControl = new GMControlTextEditBorder(widget);
	d->buffer = new GMTypoTextBuffer();
	createBufferTypoEngineIfNotExist();
	initStyles(widget);
}

GMControlTextEdit::~GMControlTextEdit()
{
	D(d);
	GM_delete(d->borderControl);
	GM_delete(d->buffer);
}

void GMControlTextEditBorder::initStyles(GMWidget* widget)
{
	D(d);
	d->borderStyle.setTexture(GMWidgetResourceManager::Skin, widget->getArea(GMTextureArea::TextEditBorderArea));
	d->borderStyle.setTextureColor(GMControlState::Normal, GMVec4(1.f, 1.f, 1.f, 1.f));
}

void GMControlTextEdit::render(GMfloat elapsed)
{
	D(d);
	if (!getVisible())
		return;

	createBufferTypoEngineIfNotExist();

	placeCaret(d->cp);
	d->borderControl->render(elapsed);

	// 计算首个能显示的字符
	GMint firstX;
	d->buffer->CPtoX(d->firstVisibleCP, false, &firstX);

	// 计算选区
	GMint caretX;
	GMint selectionStartX = 0;
	d->buffer->CPtoX(d->cp, false, &caretX);
	if (d->cp != d->selectionStartCP)
		d->buffer->CPtoX(d->selectionStartCP, false, &selectionStartX);
	else
		selectionStartX = caretX;

	GMWidget* widget = getParent();
	GMRect rcSelection;
	if (d->cp != d->selectionStartCP)
	{
		// 如果当前位置不等于选定开始的位置，则确定一个选区
		GMint selectionLeftX = caretX, selectionRightX = selectionStartX;
		if (selectionLeftX > selectionRightX)
		{
			GM_SWAP(selectionLeftX, selectionRightX);
		}

		rcSelection.x = selectionLeftX + (d->rcText.x - firstX);
		rcSelection.y = getCaretTop();
		rcSelection.width = selectionRightX - selectionLeftX;
		rcSelection.height = getCaretHeight();
		GMRect rc = GM_intersectRect(rcSelection, d->rcText);
		widget->drawRect(d->selectionBackColor, rc, true, .99f);
	}

	d->textStyle.getFontColor().setCurrent(d->textColor);

	// 闪烁光标
	blinkCaret(firstX, caretX);

	// 获取能够显示的文本长度
	calculateRenderText(firstX);

	// 不允许换行
	widget->drawText(getRenderText(), d->textStyle, d->rcText, false, false, false);
}

void GMControlTextEdit::setSize(GMint width, GMint height)
{
	D(d);
	GMControl::setSize(width, height);
	d->borderControl->setSize(width, height);
}

void GMControlTextEdit::setPosition(GMint x, GMint y)
{
	D(d);
	GMControl::setPosition(x, y);

	// border是以widget为参照，而不是以本控件为参照，所以要调整一次
	d->borderControl->setPosition(x, y);
}

bool GMControlTextEdit::onKeyDown(GMSystemKeyEvent* event)
{
	if (!getVisible() || !getEnabled())
		return false;

	D(d);
	bool handled = false;

	GMKey key = event->getKey();
	switch (key)
	{
	case GMKey_Tab:
		break;
	case GMKey_Left:
	case GMKey_Right:
	{
		moveCaret(key == GMKey_Right, !!(event->getModifier() & GMModifier_Ctrl), !!(event->getModifier() & GMModifier_Shift));
		handled = true;
		break;
	}
	case GMKey_Up:
	case GMKey_Down:
	{
		// 上下不会切换焦点
		handled = true;
		break;
	}
	case GMKey_Home:
	{
		placeCaret(0);
		if (!(event->getModifier() & GMModifier_Shift))
		{
			// 如果没有按住Shift，更新选区范围
			d->selectionStartCP = d->cp;
		}
		resetCaretBlink();
		return true;
	}
	case GMKey_End:
	{
		placeCaret(d->buffer->getLength());
		if (!(event->getModifier() & GMModifier_Shift))
		{
			// 如果没有按住Shift，更新选区范围
			d->selectionStartCP = d->cp;
		}
		resetCaretBlink();
		return true;
	}
	case GMKey_Delete:
	{
		if (d->cp != d->selectionStartCP)
		{
			// 删除一个选区
			deleteSelectionText();
			emit(textChanged);
		}
		else
		{
			if (d->buffer->removeChar(d->cp))
			{
				emit(textChanged);
			}
		}
		resetCaretBlink();
		return true;
	}
	case GMKey_Back:
	{
		if (d->selectionStartCP != d->cp)
		{
			deleteSelectionText();
		}
		else if (d->cp > 0)
		{
			placeCaret(d->cp - 1);
			d->selectionStartCP = d->cp;
			moveFirstVisibleCp(1);
			d->buffer->removeChar(d->cp);
			emit(textChanged);
			resetCaretBlink();
		}
		return true;
	}
	case GMKey_Insert:
	{
		if (event->getModifier() & GMModifier_Ctrl)
			copyToClipboard();
		else if (event->getModifier() & GMModifier_Shift)
			pasteFromClipboard();
		else
			d->insertMode = !d->insertMode;
		return true;
	}
	case GMKey_Escape:
		// 把ESC吞了，它和Ctrl+V的控制字符一样，而Ctrl+V表示粘贴。
		return true;
	}

	return handled;
}

bool GMControlTextEdit::onMouseDown(GMSystemMouseEvent* event)
{
	if (!hasFocus())
		getParent()->requestFocus(this);

	const GMPoint& pt = event->getPoint();
	if (!containsPoint(pt))
		return false;

	if (event->getModifier() & GMModifier_Shift)
		handleMouseSelect(event, false);
	else
		handleMouseSelect(event, true);

	return true;
}

bool GMControlTextEdit::onMouseDblClick(GMSystemMouseEvent* event)
{
	if (!hasFocus())
		getParent()->requestFocus(this);

	const GMPoint& pt = event->getPoint();
	if (!containsPoint(pt))
		return false;

	if (event->getModifier() & GMModifier_Shift)
		handleMouseSelect(event, false);
	else
		handleMouseSelect(event, true);

	return true;
}

bool GMControlTextEdit::onMouseUp(GMSystemMouseEvent* event)
{
	D(d);
	d->mouseDragging = false;
	GMWidget* parent = getParent();
	if (parent)
		parent->getParentWindow()->setWindowCapture(false);
	return false;
}

bool GMControlTextEdit::onMouseMove(GMSystemMouseEvent* event)
{
	D(d);
	if (d->mouseDragging)
		handleMouseSelect(event, false);
	return false;
}

bool GMControlTextEdit::onChar(GMSystemCharEvent* event)
{
	D(d);
	switch (event->getCharacter())
	{
	case GMFunctionCharacter_Ctrl_A:
	{
		selectAll();
		return true;
	}
	case GMFunctionCharacter_Ctrl_C:
	case GMFunctionCharacter_Ctrl_X:
	{
		copyToClipboard();
		if (event->getCharacter() == GMFunctionCharacter_Ctrl_X)
		{
			deleteSelectionText();
			emit(textChanged);
		}
		return true;
	}
	case GMFunctionCharacter_Ctrl_V:
	{
		pasteFromClipboard();
		return true;
	}
	case GMFunctionCharacter_Ctrl_B:
	case GMFunctionCharacter_Ctrl_D:
	case GMFunctionCharacter_Ctrl_E:
	case GMFunctionCharacter_Ctrl_F:
	case GMFunctionCharacter_Ctrl_G:
	case GMFunctionCharacter_Ctrl_H:
	case GMFunctionCharacter_Ctrl_I:
	case GMFunctionCharacter_Ctrl_J:
	case GMFunctionCharacter_Ctrl_K:
	case GMFunctionCharacter_Ctrl_L:
	case GMFunctionCharacter_Ctrl_N:
	case GMFunctionCharacter_Ctrl_O:
	case GMFunctionCharacter_Ctrl_P:
	case GMFunctionCharacter_Ctrl_Q:
	case GMFunctionCharacter_Ctrl_R:
	case GMFunctionCharacter_Ctrl_S:
	case GMFunctionCharacter_Ctrl_T:
	case GMFunctionCharacter_Ctrl_U:
	case GMFunctionCharacter_Ctrl_W:
	case GMFunctionCharacter_Ctrl_Y:
	case GMFunctionCharacter_Ctrl_Z:
	case GMFunctionCharacter_Ctrl_LeftBracket:
	case GMFunctionCharacter_Ctrl_RightBracket:
	case GMFunctionCharacter_Ctrl_Dash:
	case GMFunctionCharacter_Ctrl_Caret:
	case GMFunctionCharacter_Ctrl_Underline:
	case GMFunctionCharacter_Ctrl_Delete:
	{
		return true;
	}
	case GMFunctionCharacter_Ctrl_M:
	{
		// 按下Ctrl，说明按下了Ctrl+M，而不是按下Enter
		if (event->getModifier() & GMModifier_Ctrl)
			return true;
	}
	}

	GMwchar ch = event->getCharacter();
	if (ch == '\r')
	{
		insertCharacter(ch);
		insertCharacter('\n');
	}
	else
	{
		insertCharacter(ch);
	}

	resetCaretBlink();
	emit(textChanged);

	return true;
}

void GMControlTextEdit::onMouseEnter()
{
	D(d);
	Base::onMouseEnter();
	GMWidget* widget = getParent();
	if (widget)
		widget->getParentWindow()->setCursor(GMCursorType::IBeam);
}

void GMControlTextEdit::onMouseLeave()
{
	D(d);
	Base::onMouseEnter();
	GMWidget* widget = getParent();
	if (widget)
		widget->getParentWindow()->setCursor(GMCursorType::Arrow);
}

bool GMControlTextEdit::canHaveFocus()
{
	D(d);
	return getVisible() && getEnabled();
}

void GMControlTextEdit::setText(const GMString& text)
{
	D(d);
	d->buffer->setBuffer(text);
}

void GMControlTextEdit::setPadding(GMint x, GMint y)
{
	D(d);
	d->padding[0] = x;
	d->padding[1] = y;
	updateRect();
}

void GMControlTextEdit::createBufferTypoEngineIfNotExist()
{
	D(d);
	if (!d->buffer->getTypoEngine())
	{
		ITypoEngine* typoEngine = getParent()->getManager()->getTypoEngine();
		ITypoEngine* newInstance = nullptr;
		typoEngine->createInstance(&newInstance);
		d->buffer->setTypoEngine(newInstance);
	}
	GM_ASSERT(d->buffer->getTypoEngine());
}

void GMControlTextEdit::blinkCaret(GMint firstX, GMint caretX)
{
	D(d);
	GMfloat time = GM.getGameMachineRunningStates().elapsedTime;
	if (time - d->lastBlink >= d->deltaBlink)
	{
		d->caretOn = !d->caretOn;
		d->lastBlink = time;
	}

	if (hasFocus() && d->caretOn && d->showCaret)
		renderCaret(firstX, caretX);
}

void GMControlTextEdit::placeCaret(GMint cp)
{
	D(d);
	GM_ASSERT(cp >= 0);
	if (cp > d->buffer->getLength())
		cp = d->buffer->getLength();
	d->cp = cp;

	GMint firstX, x, x2;
	d->buffer->CPtoX(d->firstVisibleCP, false, &firstX);
	d->buffer->CPtoX(cp, false, &x); //lead
	if (cp == d->buffer->getLength())
		x2 = x;
	else
		d->buffer->CPtoX(cp, true, &x2); //trail

	if (x < firstX)
	{
		// 此时字符所在的位置比可见位置小，我们需要调整滚动条到其可见
		d->firstVisibleCP = cp;
	}
	else if (x2 > firstX + d->rcText.width)
	{
		GMint xNewLeft = x2 - d->rcText.width;
		GMint cpNewFirst;
		bool newTrail;
		d->buffer->XtoCP(xNewLeft, &cpNewFirst, &newTrail);
		GMint xNewFirst;
		d->buffer->CPtoX(cpNewFirst, false, &xNewFirst);
		if (xNewFirst < xNewLeft)
			++cpNewFirst;

		d->firstVisibleCP = cpNewFirst;
	}
}

void GMControlTextEdit::moveCaret(bool next, bool newItem, bool select)
{
	D(d);
	if (newItem && next)
	{
		d->buffer->getNextItemPos(d->cp, &d->cp);
		placeCaret(d->cp);
	}
	else if (newItem && !next)
	{
		d->buffer->getPriorItemPos(d->cp, &d->cp);
		placeCaret(d->cp);
	}
	else if (next && d->cp < d->buffer->getLength())
	{
		placeCaret(d->cp + 1);
	}
	else if (!next && d->cp > 0)
	{
		placeCaret(d->cp - 1);
	}
	if (!select)
	{
		d->selectionStartCP = d->cp;
	}
	resetCaretBlink();
}

void GMControlTextEdit::deleteSelectionText()
{
	D(d);
	GMint firstCp = Min(d->cp, d->selectionStartCP);
	GMint lastCp = Max(d->cp, d->selectionStartCP);
	placeCaret(firstCp);
	d->selectionStartCP = d->cp;
	d->buffer->removeChars(firstCp, lastCp);
	moveFirstVisibleCp(lastCp - firstCp);
}

void GMControlTextEdit::selectAll()
{
	D(d);
	d->selectionStartCP = 0;
	placeCaret(d->buffer->getLength());
}

void GMControlTextEdit::resetCaretBlink()
{
	D(d);
	d->lastBlink = GM.getGameMachineRunningStates().elapsedTime;
	d->caretOn = true;
}

void GMControlTextEdit::copyToClipboard()
{
	D(d);
	GMBuffer clipboardBuffer;
	auto selectionStart = d->selectionStartCP;
	auto selectionEnd = d->cp;
	if (selectionStart > selectionEnd)
		GM_SWAP(selectionStart, selectionEnd);

	GMString subString = d->buffer->getBuffer().substr(selectionStart, selectionEnd - selectionStart);
	clipboardBuffer.size = (subString.length() + 1) * sizeof(decltype(subString.c_str()[0])); //不能忘记\0
	clipboardBuffer.buffer = const_cast<GMbyte*>(reinterpret_cast<const GMbyte*>(subString.c_str()));
	clipboardBuffer.needRelease = false;
	GMClipboard::setData(GMClipboardMIME::UnicodeText, clipboardBuffer);
}

void GMControlTextEdit::pasteFromClipboard()
{
	D(d);
	deleteSelectionText();
	GMBuffer clipboardBuffer = GMClipboard::getData(GMClipboardMIME::UnicodeText);
	GMString string(reinterpret_cast<GMwchar*>(clipboardBuffer.buffer));
	string = string.replace("\r", "").replace("\n", "").replace("\t", " ");
	if (d->buffer->insertString(d->cp, string))
	{
		placeCaret(d->cp + string.length());
		d->selectionStartCP = d->cp;
	}
}

void GMControlTextEdit::handleMouseSelect(GMSystemMouseEvent* event, bool selectStart)
{
	D(d);
	d->mouseDragging = true;
	GMWidget* parent = getParent();
	if (parent)
		parent->getParentWindow()->setWindowCapture(true);
	handleMouseCaret(event->getPoint(), selectStart);
}

void GMControlTextEdit::initStyles(GMWidget* widget)
{
	D(d);
	GMStyle textStyle;
	textStyle.setFont(0);
	textStyle.setFontColor(GMVec4(0, 0, 0, 1));
	d->textStyle = textStyle;
}

void GMControlTextEdit::handleMouseCaret(const GMPoint& pt, bool selectStart)
{
	D(d);
	GMint xFirst;
	d->buffer->CPtoX(d->firstVisibleCP, false, &xFirst);

	GMint cp;
	bool trail;
	if (d->buffer->XtoCP(pt.x - d->rcText.x + xFirst, &cp, &trail))
	{
		if (trail && cp < d->buffer->getLength())
			placeCaret(cp + 1);
		else
			placeCaret(cp);

		if (selectStart)
			d->selectionStartCP = d->cp;
		resetCaretBlink();
	}
}

void GMControlTextEdit::updateRect()
{
	GMControl::updateRect();
	
	// 更新文本框绘制文本位置，并且同步到排版buffer
	D(d);
	d->rcText = boundingRect();
	d->rcText.x += d->borderWidth + d->padding[0];
	d->rcText.width -= (d->borderWidth + d->padding[0]) * 2;
	d->rcText.y += d->borderWidth + d->padding[1];
	d->rcText.height -= (d->borderWidth + d->padding[1]) * 2;
	d->buffer->setSize(d->rcText);
}

void GMControlTextEdit::calculateRenderText(GMint firstX)
{
	D(d);
	// 获取能够显示的文本长度
	const GMString& text = d->buffer->getBuffer();
	GMint lastCP;
	bool lastTrail;
	d->buffer->XtoCP(firstX + d->rcText.width, &lastCP, &lastTrail);
	if (lastTrail)
		d->renderText = text.substr(d->firstVisibleCP, lastCP - d->firstVisibleCP + 1);
	else if (!lastTrail)
		d->renderText = text.substr(d->firstVisibleCP, lastCP - d->firstVisibleCP);
}

void GMControlTextEdit::insertCharacter(GMwchar ch)
{
	D(d);
	if (ch == '\r' || ch == '\n') //吞掉换行
		return;

	if (d->selectionStartCP != d->cp)
		deleteSelectionText();

	if (!d->insertMode && d->cp < d->buffer->getLength())
	{
		d->buffer->setChar(d->cp, ch);
		placeCaret(d->cp + 1);
		d->selectionStartCP = d->cp;
	}
	else
	{
		if (d->buffer->insertChar(d->cp, ch))
		{
			placeCaret(d->cp + 1);
			d->selectionStartCP = d->cp;
		}
	}
}

void GMControlTextEdit::renderCaret(GMint firstX, GMint caretX)
{
	D(d);
	GMRect rc = {
		d->rcText.x - firstX + caretX - 1,
		getCaretTop(),
		2,
		getCaretHeight()
	};
	GMVec4* caretColor = nullptr;
	if (!d->insertMode)
	{
		// 改写模式
		GMint rightEdgeX;
		d->buffer->CPtoX(d->cp, true, &rightEdgeX);
		rc.width = rightEdgeX - caretX;
		if (rc.width <= 0)
			rc.width = 8; //给一个最小的光标宽度
		rc.height = 2;
		rc.y = getCaretTop() + getCaretHeight() - 2;
		caretColor = &d->selectionBackColor;
	}
	else
	{
		caretColor = &d->caretColor;
	}

	GMWidget* widget = getParent();
	widget->drawRect(*caretColor, rc, true, .99f);
}

GMint GMControlTextEdit::getCaretHeight()
{
	D(d);
	return d->buffer->getLineHeight() + 3;
}

GMint GMControlTextEdit::getCaretTop()
{
	D(d);
	return d->rcText.y;
}

void GMControlTextEdit::moveFirstVisibleCp(GMint distance)
{
	D(d);
	if (d->firstVisibleCP > distance)
		d->firstVisibleCP -= distance;
	else
		d->firstVisibleCP = 0;
}

GMControlTextArea::GMControlTextArea(GMWidget* widget)
	: Base(widget)
{
	D(d);
	D_BASE(db, Base);
	GM_delete(db->buffer);
	db->buffer = d->buffer = new GMMultiLineTypoTextBuffer();
	createBufferTypoEngineIfNotExist();
}

void GMControlTextArea::render(GMfloat elapsed)
{
	D(d);
	D_BASE(db, Base);
	if (!getVisible())
		return;

	createBufferTypoEngineIfNotExist();

	placeCaret(db->cp);
	db->borderControl->render(elapsed);

	// 计算首个能显示的字符
	GMint firstX, firstY;
	d->buffer->CPtoXY(db->firstVisibleCP, false, &firstX, &firstY);

	// 计算选区
	GMint caretX;
	GMint caretY;
	GMint caretSelectionX = 0;
	GMint caretSelectionY = 0;

	d->buffer->CPtoXY(db->cp, false, &caretX, &caretY);
	if (db->cp != db->selectionStartCP)
	{
		d->buffer->CPtoXY(db->selectionStartCP, false, &caretSelectionX, &caretSelectionY);
	}
	else
	{
		caretSelectionX = caretX;
		caretSelectionY = caretY;
	}

	GMWidget* widget = getParent();
	GMRect rcSelection;
	GMint selectionLeftLineNo = d->buffer->CPToLineNumber(db->selectionStartCP);
	GMint selectionRightLineNo = d->buffer->CPToLineNumber(db->cp);
	if (selectionLeftLineNo > selectionRightLineNo)
		GM_SWAP(selectionLeftLineNo, selectionRightLineNo);

	if (db->cp != db->selectionStartCP)
	{
		if (selectionLeftLineNo == selectionRightLineNo)
		{
			GMint selectionStartX = caretX, selectionEndX = caretSelectionX;
			if (selectionStartX > selectionEndX)
				GM_SWAP(selectionStartX, selectionEndX);

			// 被选中的块位于同一行
			rcSelection.x = selectionStartX + (db->rcText.x - firstX);
			rcSelection.y = getCaretTop();
			rcSelection.width = selectionEndX - selectionStartX;
			rcSelection.height = getCaretHeight();
			GMRect rc = GM_intersectRect(rcSelection, db->rcText);
			widget->drawRect(db->selectionBackColor, rc, true, .99f);
		}
		else
		{
			// 跨行的情况，需要绘制2~3个矩形
			GM_ASSERT(selectionRightLineNo > selectionLeftLineNo);
			auto selectionStartCP = db->selectionStartCP, // 选区开始的CP
				selectionEndCP = db->cp; //选区结束的CP

			if (selectionStartCP > selectionEndCP)
				GM_SWAP(selectionStartCP, selectionEndCP);

			GMint selectionStartX, selectionEndX;
			GMint selectionStartY, selectionEndY;
			d->buffer->CPtoXY(selectionStartCP, false, &selectionStartX, &selectionStartY);
			d->buffer->CPtoXY(selectionEndCP, false, &selectionEndX, &selectionEndY);

			// 绘制最上面的矩形
			GMint firstLineLastCp = d->buffer->findLastCPInOneLine(selectionStartCP);
			{
				// 获取选择的首行的最后一个CP

				// 获取CP的X和Y
				GMint selStartLastX, selStartLastY;
				d->buffer->CPtoXY(firstLineLastCp, true, &selStartLastX, &selStartLastY);

				setCaretTopRelative(selStartLastY);
				rcSelection.x = selectionStartX + (db->rcText.x - firstX);
				rcSelection.y = getCaretTop();
				rcSelection.width = selStartLastX - selectionStartX;
				rcSelection.height = getCaretHeight();
				GMRect rc = GM_intersectRect(rcSelection, db->rcText);
				widget->drawRect(db->selectionBackColor, rc, true, .99f);
			}

			// 绘制最下方的矩形
			GMint lastLineFirstCp = 0;
			{
				// 获取选择的尾行的首个CP
				lastLineFirstCp = d->buffer->findFirstCPInOneLine(selectionEndCP);

				// 获取CP的X和Y
				GMint selEndFirstX, selEndFirstY;
				d->buffer->CPtoXY(lastLineFirstCp, false, &selEndFirstX, &selEndFirstY);

				setCaretTopRelative(selEndFirstY);
				rcSelection.x = selEndFirstX + (db->rcText.x - firstX);
				rcSelection.y = getCaretTop();
				rcSelection.width = selectionEndX - selEndFirstX;
				rcSelection.height = getCaretHeight();
				GMRect rc = GM_intersectRect(rcSelection, db->rcText);
				widget->drawRect(db->selectionBackColor, rc, true, .99f);
			}

			if (selectionRightLineNo - selectionLeftLineNo > 1)
			{
				// 如果中间还有内容，中间全选内容
				GMint nextLineFirstCp = firstLineLastCp + 1;
				GMint currentLineLastCp = 0;
				while (nextLineFirstCp <= lastLineFirstCp - 1) //lastLineFirstCp - 1表示倒数第二行的最后一个CP
				{
					currentLineLastCp = d->buffer->findLastCPInOneLine(nextLineFirstCp);
					// 现在获取了这一行的CP范围，可以开始绘制选区了
					GMint leftX, leftY, rightX, rightY;
					d->buffer->CPtoXY(nextLineFirstCp, false, &leftX, &leftY);
					d->buffer->CPtoXY(currentLineLastCp, false, &rightX, &rightY);

					setCaretTopRelative(leftY);
					rcSelection.x = leftX + (db->rcText.x - firstX);
					rcSelection.y = getCaretTop();
					rcSelection.width = rightX - leftX;
					rcSelection.height = getCaretHeight();
					// 如果是个空行导致选区宽度小于一个值，我们将它设置一个最小宽度
					if (rcSelection.width < 5)
						rcSelection.width = 10;
					GMRect rc = GM_intersectRect(rcSelection, db->rcText);
					widget->drawRect(db->selectionBackColor, rc, true, .99f);

					nextLineFirstCp = currentLineLastCp + 1;
				}
			}
		}
	}

	setCaretTopRelative(caretY);
	db->textStyle.getFontColor().setCurrent(db->textColor);

	// 闪烁光标
	blinkCaret(firstX, caretX);

	// 获取能够显示的文本长度
	calculateRenderText(firstX);
	widget->drawText(getRenderText(), db->textStyle, db->rcText, false, false, true, d->buffer->getLineSpacing());
}

void GMControlTextArea::pasteFromClipboard()
{
	D_BASE(d, Base);
	deleteSelectionText();
	GMBuffer clipboardBuffer = GMClipboard::getData(GMClipboardMIME::UnicodeText);
	GMString string(reinterpret_cast<GMwchar*>(clipboardBuffer.buffer));
	if (d->buffer->insertString(d->cp, string))
	{
		placeCaret(d->cp + string.length());
		d->selectionStartCP = d->cp;
	}
}

void GMControlTextArea::insertCharacter(GMwchar ch)
{
	D_BASE(d, Base);
	if (d->selectionStartCP != d->cp)
		deleteSelectionText();

	if (!d->insertMode && d->cp < d->buffer->getLength())
	{
		d->buffer->setChar(d->cp, ch);
		placeCaret(d->cp + 1);
		d->selectionStartCP = d->cp;
	}
	else
	{
		if (d->buffer->insertChar(d->cp, ch))
		{
			placeCaret(d->cp + 1);
			d->selectionStartCP = d->cp;
		}
	}
}

void GMControlTextArea::setSize(GMint width, GMint height)
{
	D_BASE(d, Base);
	Base::setSize(width, height);
	GMRect rc = { 0, 0, width, height };
	d->buffer->setSize(rc);
}

GMint GMControlTextArea::getCaretTop()
{
	D(d);
	D_BASE(db, Base);
	return d->caretTopRelative + db->rcText.y;
}

void GMControlTextArea::handleMouseCaret(const GMPoint& pt, bool selectStart)
{
	D(d);
	D_BASE(db, Base);
	GMint xFirst, yFirst;
	d->buffer->CPtoXY(db->firstVisibleCP, false, &xFirst, &yFirst);

	GMint cp;
	if (d->buffer->XYtoCP(pt.x - db->rcText.x + xFirst, pt.y - db->rcText.y + yFirst, &cp))
	{
		placeCaret(cp);

		if (selectStart)
			db->selectionStartCP = db->cp;
		resetCaretBlink();
	}
}

void GMControlTextArea::setLineSpacing(GMint lineSpacing) GM_NOEXCEPT
{
	D(d);
	d->buffer->setLineSpacing(lineSpacing);
}