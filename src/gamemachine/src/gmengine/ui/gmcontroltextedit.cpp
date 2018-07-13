#include "stdafx.h"
#include <gmtypoengine.h>
#include "gmcontroltextedit.h"
#include "foundation/gamemachine.h"

#ifndef WHEEL_DELTA
#define WHEEL_DELTA 120
#endif

#define GM_WHEEL_DELTA WHEEL_DELTA

BEGIN_NS

GM_DEFINE_SIGNAL(GMControlTextEdit::textChanged)
GM_PRIVATE_OBJECT(GMMultiLineTypoTextBuffer)
{
	GMint lineSpacing = 5;
	GMint lineHeight = 0;
};

GM_PRIVATE_OBJECT(GMControlTextControlTransactionAtom)
{
	GMint cpStart = 0;
	GMint cpEnd = 0;
	GMint cpLastStart = 0;
	GMint cpLastEnd = 0;
	GMControlTextEdit* textEdit = nullptr;
};

class GMControlTextControlTransactionAtom : public ITransactionAtom
{
	GM_DECLARE_PRIVATE(GMControlTextControlTransactionAtom)

public:
	GMControlTextControlTransactionAtom(
		GMControlTextEdit* textEdit,
		GMint cpStart,
		GMint cpEnd,
		GMint cpLastStart,
		GMint cpLastEnd
	);

	virtual void execute() override;
	virtual void unexecute() override;
};

GMControlTextControlTransactionAtom::GMControlTextControlTransactionAtom(
	GMControlTextEdit* textEdit,
	GMint cpStart,
	GMint cpEnd,
	GMint cpLastStart,
	GMint cpLastEnd
)
{
	D(d);
	d->textEdit = textEdit;
	d->cpStart = cpStart;
	d->cpEnd = cpEnd;
	d->cpLastStart = cpLastStart;
	d->cpLastEnd = cpLastEnd;
}

void GMControlTextControlTransactionAtom::execute()
{
	D(d);
	d->textEdit->placeCaret(d->cpStart, true);
	d->textEdit->placeSelectionStart(d->cpEnd);
}

void GMControlTextControlTransactionAtom::unexecute()
{
	D(d);
	d->textEdit->placeCaret(d->cpLastStart, true);
	d->textEdit->placeSelectionStart(d->cpLastEnd);
}

class GMMultiLineTypoTextBuffer : public GMTypoTextBuffer
{
	GM_DECLARE_PRIVATE_AND_BASE(GMMultiLineTypoTextBuffer, GMTypoTextBuffer)

public:
	GMMultiLineTypoTextBuffer() = default;

public:
	virtual void analyze(GMint start) override;
	virtual bool CPtoX(GMint cp, bool trail, GMint* x) { GM_ASSERT(false); return false; }
	virtual bool XtoCP(GMint x, GMint* cp, bool* trail) { GM_ASSERT(false); return false; }
	virtual bool CPtoXY(GMint cp, bool trail, GMint* x, GMint* y);
	virtual bool CPtoMidXY(GMint cp, GMint* x, GMint* y);
	virtual bool XYtoCP(GMint x, GMint y, GMint* cp);
	
public:
	GMint CPToLineNumber(GMint cp);
	GMint findFirstCPInOneLine(GMint cp);
	GMint findLastCPInOneLine(GMint cp);
	bool isNewLine(GMint cp);

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

	inline void setLineHeight(GMint lineHeight) GM_NOEXCEPT
	{
		D(d);
		d->lineHeight = lineHeight;
	}
};

void GMMultiLineTypoTextBuffer::analyze(GMint start)
{
	D(d);
	D_BASE(db, Base);
	GMTypoOptions options;
	options.typoArea = db->rc;
	options.newline = true;
	options.plainText = isPlainText();
	options.lineSpacing = d->lineSpacing;
	db->engine->setLineHeight(d->lineHeight);
	db->engine->begin(db->buffer, options, start);
	db->dirty = false;
}

bool GMMultiLineTypoTextBuffer::CPtoXY(GMint cp, bool trail, GMint* x, GMint* y)
{
	D_BASE(d, Base);
	if (cp < 0)
	{
		if (x)
			*x = 0;

		if (y)
			*y = 0;
		return true;
	}

	if (d->dirty)
		analyze(cp);

	decltype(auto) r = d->engine->getResults().results;
	if (cp >= getLength())
	{
		// 越界分两种情况。如果最后一个字符是换行或仅有一个回车符，那么获取的坐标为下一行，否则获取最后一个字符的位置
		// 由于缓存最末尾有个EOF，因此要getLength() - 1
		if ( (getLength() > 1 && r[getLength() - 1].newLineOrEOFSeparator) ||
			 (getLength() == 1 && r[0].newLineOrEOFSeparator)
		)
		{
			CPtoXY(getLength() - 1, true, nullptr, y);
			if (x)
				*x = 0;
			if (y)
				*y = *y + getLineHeight() + getLineSpacing();
			return true;
		}

		return CPtoXY(getLength() - 1, true, x, y);
	}

	if (x)
	{
		if (trail)
			*x = r[cp].x + r[cp].advance;
		else
			*x = r[cp].x;
	}

	if (y)
		*y = r[cp].y;

	return true;
}

bool GMMultiLineTypoTextBuffer::CPtoMidXY(GMint cp, GMint* x, GMint* y)
{
	D_BASE(d, Base);
	if (cp < 0)
	{
		if (x)
			*x = 0;

		if (y)
			*y = 0;
		return true;
	}

	if (d->dirty)
		analyze(cp);

	decltype(auto) r = d->engine->getResults().results;
	if (cp >= getLength())
	{
		// 越界分两种情况。如果最后一个字符是换行或仅有一个回车符，那么获取的坐标为下一行，否则获取最后一个字符的位置
		// 由于缓存最末尾有个EOF，因此要getLength() - 1
		if ((getLength() > 1 && r[getLength() - 1].newLineOrEOFSeparator) ||
			(getLength() == 1 && r[0].newLineOrEOFSeparator)
			)
		{
			CPtoMidXY(getLength() - 1, nullptr, y);
			if (x)
				*x = 0;
			if (y)
				*y = *y + getLineHeight() + getLineSpacing();
			return true;
		}

		return CPtoMidXY(getLength() - 1, x, y);
	}

	if (x)
		*x = r[cp].x + r[cp].advance * .5f;
	if (y)
		*y = r[cp].y + r[cp].height * .5f;
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
		analyze(0);

	decltype(auto) r = d->engine->getResults().results;
	if (r.empty())
	{
		*cp = 0;
		return true;
	}

	// r表示排版结果，最后为一个eof符号
	// 认为字符是紧密排列的
	const GMPoint pt = { x, y };
	for (GMsize_t i = 0; i < r.size() - 1; ++i)
	{
		GMRect glyphRc = {
			static_cast<GMint>(r[i].x - r[i].bearingX),
			static_cast<GMint>(r[i].y),
			static_cast<GMint>(r[i].advance),
			static_cast<GMint>(getLineHeight() + getLineSpacing())
		};

		if (GM_inRect(glyphRc, pt))
		{
			*cp = i;
			return true;
		}
	}

	// 如果没有选中某个字符，那么选择这一行末尾
	GMint currentLine = 0;
	GMint rowX, rowY;
	for (GMsize_t i = 0; i < r.size() - 1; ++i)
	{
		if (currentLine == r[i].lineNo)
			continue;

		currentLine = r[i].lineNo;
		CPtoXY(i, true, &rowX, &rowY);

		if (r[i].y <= pt.y && pt.y <= r[i].y + getLineHeight() + getLineSpacing())
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
		analyze(cp);

	decltype(auto) r = d->engine->getResults().results;
	if (cp > static_cast<GMint>(r.size()))
		return r[r.size() - 1].lineNo;

	return r[cp].lineNo;
}

GMint GMMultiLineTypoTextBuffer::findFirstCPInOneLine(GMint cp)
{
	D_BASE(d, Base);
	if (d->dirty)
		analyze(cp);

	decltype(auto) r = d->engine->getResults().results;
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
		analyze(cp);

	decltype(auto) r = d->engine->getResults().results;
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

bool GMMultiLineTypoTextBuffer::isNewLine(GMint cp)
{
	D_BASE(d, Base);
	decltype(auto) r = d->engine->getResults().results;
	GM_ASSERT(cp < static_cast<GMint>(r.size()));
	return r[cp].newLineOrEOFSeparator;
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
	d->borderControl = gm_makeOwnedPtr<GMControlTextEditBorder>(widget);
	d->buffer = gm_makeOwnedPtr<GMTypoTextBuffer>();
	createBufferTypoEngineIfNotExist();
	initStyles(widget);
}

GMControlTextEdit::~GMControlTextEdit() = default;

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
	setBufferRenderRange(firstX);

	// 不允许换行
	widget->drawText(d->buffer.get(), d->textStyle, d->rcText, false);
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

GMStyle& GMControlTextEdit::getStyle(GMControl::StyleType style)
{
	D(d);
	switch (style)
	{
	case GMControlTextEdit::TextStyle:
		break;
	default:
		GM_ASSERT(false);
		break;
	}
	return Base::getStyle(style);
}

void GMControlTextEdit::onFocusOut()
{
	D(d);
	Base::onFocusOut();
	GMWidget* widget = getParent();
	if (widget)
		widget->getParentWindow()->setCursor(GMCursorType::Arrow);
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
		handled = onKey_Tab(event);
		break;
	case GMKey_Left:
	case GMKey_Right:
	{
		handled = onKey_LeftRight(event);
		break;
	}
	case GMKey_Up:
	case GMKey_Down:
	{
		handled = onKey_UpDown(event);
		break;
	}
	case GMKey_Home:
	case GMKey_End:
	{
		handled = onKey_HomeEnd(event);
		break;
	}
	case GMKey_Delete:
	{
		handled = onKey_Delete(event);
		break;
	}
	case GMKey_Back:
	{
		handled = onKey_Back(event);
		break;
	}
	case GMKey_Insert:
	{
		handled = onKey_Insert(event);
		break;
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
	if (GM_inRect(d->rcText, event->getPoint()))
	{
		GMWidget* widget = getParent();
		if (widget)
			widget->getParentWindow()->setCursor(GMCursorType::IBeam);
	}

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
	{
		//d->buffer->redo();
		return true;
	}
	case GMFunctionCharacter_Ctrl_Z:
	{
		//d->buffer->undo();
		return true;
	}
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
	Base::onMouseLeave();
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

bool GMControlTextEdit::onKey_Tab(GMSystemKeyEvent* event)
{
	return true;
}

bool GMControlTextEdit::onKey_LeftRight(GMSystemKeyEvent* event)
{
	GMKey key = event->getKey();
	moveCaret(key == GMKey_Right, !!(event->getModifier() & GMModifier_Ctrl), !!(event->getModifier() & GMModifier_Shift));
	return true;
}

bool GMControlTextEdit::onKey_UpDown(GMSystemKeyEvent* event)
{
	// 上下不会切换焦点
	return true;
}

bool GMControlTextEdit::onKey_HomeEnd(GMSystemKeyEvent* event)
{
	D(d);
	GMKey key = event->getKey();
	if (key == GMKey_Home)
	{
		placeCaret(0);
	}
	else
	{
		GM_ASSERT(key == GMKey_End);
		placeCaret(d->buffer->getLength());
	}

	if (!(event->getModifier() & GMModifier_Shift))
	{
		// 如果没有按住Shift，更新选区范围
		d->selectionStartCP = d->cp;
	}
	resetCaretBlink();
	return true;
}

bool GMControlTextEdit::onKey_Delete(GMSystemKeyEvent* event)
{
	D(d);
	if (d->cp != d->selectionStartCP)
	{
		// 删除一个选区
		deleteSelectionText();
		emit(textChanged);
	}
	else
	{
		// 要考虑删除\r\n的情况，如果删除的是\r，且之后为\n，则一并删除

		if (d->buffer->removeChar(d->cp))
		{
			d->buffer->analyze(d->cp);
			emit(textChanged);
		}
	}
	resetCaretBlink();
	return true;
}

bool GMControlTextEdit::onKey_Back(GMSystemKeyEvent* event)
{
	D(d);
	if (d->selectionStartCP != d->cp)
	{
		deleteSelectionText();
		emit(textChanged);
	}
	else if (d->cp > 0)
	{
		placeCaret(d->cp - 1);
		d->selectionStartCP = d->cp;
		moveFirstVisibleCp(1);
		if (d->buffer->removeChar(d->cp))
		{
			d->buffer->analyze(d->cp);
			emit(textChanged);
		}

		resetCaretBlink();
	}
	return true;
}

bool GMControlTextEdit::onKey_Insert(GMSystemKeyEvent* event)
{
	D(d);
	if (event->getModifier() & GMModifier_Ctrl)
		copyToClipboard();
	else if (event->getModifier() & GMModifier_Shift)
		pasteFromClipboard();
	else
		d->insertMode = !d->insertMode;
	return true;
}

const GMString& GMControlTextEdit::getText() GM_NOEXCEPT
{
	D(d);
	return d->buffer->getBuffer();
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
	GMfloat time = GM.getRunningStates().elapsedTime;
	if (time - d->lastBlink >= d->deltaBlink)
	{
		d->caretOn = !d->caretOn;
		d->lastBlink = time;
	}

	if (hasFocus() && d->caretOn && d->showCaret)
		renderCaret(firstX, caretX);
}

void GMControlTextEdit::placeCaret(GMint cp, bool adjustVisibleCP)
{
	D(d);
	GM_ASSERT(cp >= 0);
	if (cp > d->buffer->getLength())
		cp = d->buffer->getLength();
	d->cp = cp;

	if (adjustVisibleCP)
	{
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
}

void GMControlTextEdit::adjustInsertModeRect(REF GMRect& caretRc, GMint caretX)
{
	D(d);
	GMint rightEdgeX;
	d->buffer->CPtoX(d->cp, true, &rightEdgeX);
	caretRc.width = rightEdgeX - caretX;
	if (caretRc.width <= 0)
		caretRc.width = 8; //给一个最小的光标宽度
	caretRc.height = 2;
	caretRc.y = getCaretTop() + getCaretHeight() - 2;
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
	if (d->buffer->removeChars(firstCp, lastCp))
	{
		moveFirstVisibleCp(lastCp - firstCp);
		d->buffer->analyze(firstCp);
	}
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
	d->lastBlink = GM.getRunningStates().elapsedTime;
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

	GMString subString = GMConvertion::toCurrentEnvironmentString(d->buffer->getBuffer().substr(selectionStart, selectionEnd - selectionStart));
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

		// 再次做一次微调，如果鼠标坐落在字符右半边区域，选中下一个字符。
		decltype(auto) charResult = d->buffer->getTypoEngine()->getResults().results[cp];
		auto diffFromLead = pt.x - d->rcText.x + xFirst - charResult.x;
		// 如果超过了自身字符一半的宽度，认为是选择了下一个字符
		if (diffFromLead > charResult.advance / 2)
			placeCaret(cp + 1);

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
	d->rcText = getBoundingRect();
	d->rcText.x += d->borderWidth + d->padding[0];
	d->rcText.width -= (d->borderWidth + d->padding[0]) * 2;
	d->rcText.y += d->borderWidth + d->padding[1];
	d->rcText.height -= (d->borderWidth + d->padding[1]) * 2;
	d->buffer->setSize(d->rcText);
}

void GMControlTextEdit::insertCharacter(GMwchar ch)
{
	D(d);
	if (ch == '\r' || ch == '\n') //吞掉换行
		return;

	// 使光标出现在视线范围
	placeCaret(d->cp, true);

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
		d->rcText.x + caretX - firstX - 1,
		getCaretTop(),
		2,
		getCaretHeight()
	};
	GMVec4* caretColor = nullptr;
	if (!d->insertMode)
		// 改写模式
		adjustInsertModeRect(rc, caretX);

	caretColor = &d->caretColor;
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

void GMControlTextEdit::setBufferRenderRange(GMint xFirst)
{
	D(d);
	// 获取能够显示的文本长度
	GMint lastCP;
	bool lastTrail;
	d->buffer->XtoCP(xFirst + d->rcText.width, &lastCP, &lastTrail);
	if (lastTrail)
		d->buffer->setRenderRange(d->firstVisibleCP, lastCP + 1);
	else
		d->buffer->setRenderRange(d->firstVisibleCP, lastCP);
}

GMControlTextArea::GMControlTextArea(GMWidget* widget)
	: Base(widget)
{
	D(d);
	D_BASE(db, Base);
	d->buffer = new GMMultiLineTypoTextBuffer();
	db->buffer.reset(d->buffer);
	setLineHeight(16);
	createBufferTypoEngineIfNotExist();
}

void GMControlTextArea::render(GMfloat elapsed)
{
	D(d);
	D_BASE(db, Base);
	if (!getVisible())
		return;

	createBufferTypoEngineIfNotExist();

	placeCaret(db->cp, false);
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


	// 绘制一个模板区域，防止光标、选中区域和文字超出渲染区域
	GMWidget* widget = getParent();
	GMRect expandedRcText = expandStencilRect(db->rcText); // 稍微扩大一下渲染区域，不然看起来很丑。
	widget->drawStencil(expandedRcText, .99f);
	widget->useStencil(true);

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
			GMRect rc = GM_intersectRect(rcSelection, expandedRcText);
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

			// 如果是个空行导致选区宽度小于一个值，我们将它设置一个最小宽度
			auto minSelectionRect = [](GMRect& selection) {
				if (selection.width < 5)
					selection.width = 10;
			};

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
				if (firstLineLastCp == d->buffer->findLastCPInOneLine(firstLineLastCp))
					minSelectionRect(rcSelection);
				GMRect rc = GM_intersectRect(rcSelection, expandedRcText);
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
				if (lastLineFirstCp == d->buffer->findLastCPInOneLine(lastLineFirstCp))
					minSelectionRect(rcSelection);
				GMRect rc = GM_intersectRect(rcSelection, expandedRcText);
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
					if (currentLineLastCp == d->buffer->findLastCPInOneLine(currentLineLastCp))
						minSelectionRect(rcSelection);
					GMRect rc = GM_intersectRect(rcSelection, expandedRcText);
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
	setBufferRenderRange(firstX, firstY);

	// 绘制文本
	widget->drawText(d->buffer, db->textStyle, adjustRectByScrollOffset(db->rcText), false);

	// 结束模板区域
	widget->endStencil();

	if (d->hasScrollBar && d->scrollBar)
	{
		d->scrollBar->render(elapsed);
	}
}

void GMControlTextArea::pasteFromClipboard()
{
	D_BASE(d, Base);
	deleteSelectionText();
	GMBuffer clipboardBuffer = GMClipboard::getData(GMClipboardMIME::UnicodeText);
	GMString string(reinterpret_cast<GMwchar*>(clipboardBuffer.buffer));
	string = GMConvertion::toCurrentEnvironmentString(string).replace("\t", " ");
	if (d->buffer->insertString(d->cp, string))
	{
		placeCaret(d->cp + string.length(), false);
		d->selectionStartCP = d->cp;
	}
}

void GMControlTextArea::insertCharacter(GMwchar ch)
{
	D_BASE(d, Base);
	// 使光标出现在视线范围
	placeCaret(d->cp, true);

	if (d->selectionStartCP != d->cp)
		deleteSelectionText();

	if (!d->insertMode && d->cp < d->buffer->getLength())
	{
		d->buffer->setChar(d->cp, ch);
		placeCaret(d->cp + 1, false);
		d->selectionStartCP = d->cp;
	}
	else
	{
		if (d->buffer->insertChar(d->cp, ch))
		{
			placeCaret(d->cp + 1, false);
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
	GMint yFirst = 0;
	d->buffer->CPtoXY(db->firstVisibleCP, false, nullptr, &yFirst);
	return d->caretTopRelative + db->rcText.y + d->scrollOffset - yFirst;
}

GMint GMControlTextArea::getCaretHeight()
{
	D(d);
	return d->buffer->getLineHeight() + d->buffer->getLineSpacing();
}

void GMControlTextArea::handleMouseCaret(const GMPoint& pt, bool selectStart)
{
	D(d);
	D_BASE(db, Base);
	GMint xFirst, yFirst;
	d->buffer->CPtoXY(db->firstVisibleCP, false, &xFirst, &yFirst);

	GMint cp;
	GMPoint adjustedPt = {
		pt.x,
		pt.y - d->scrollOffset
	};

	if (d->buffer->XYtoCP(adjustedPt.x - db->rcText.x + xFirst, adjustedPt.y - db->rcText.y + yFirst, &cp))
	{
		placeCaret(cp, true);

		// 再次做一次微调，如果鼠标坐落在字符右半边区域，选中下一个字符。
		decltype(auto) charResult = d->buffer->getTypoEngine()->getResults().results[cp];
		auto diffFromLead = pt.x - db->rcText.x + xFirst - charResult.x;
		// 如果自身并不是本行最后一个字符，且超过了自身字符一半的宽度，认为是选择了下一个字符
		if (d->buffer->findLastCPInOneLine(cp) != cp && diffFromLead > charResult.advance / 2)
			placeCaret(cp + 1, true);

		if (selectStart)
			db->selectionStartCP = db->cp;
		resetCaretBlink();
	}
}

void GMControlTextArea::setScrollBar(bool scrollBar)
{
	D(d);
	if (d->hasScrollBar != scrollBar)
	{
		d->hasScrollBar = scrollBar;
		updateScrollBar();
	}
}

void GMControlTextArea::setLineSpacing(GMint lineSpacing) GM_NOEXCEPT
{
	D(d);
	d->buffer->setLineSpacing(lineSpacing);
}

void GMControlTextArea::setLineHeight(GMint lineHeight) GM_NOEXCEPT
{
	D(d);
	d->buffer->setLineHeight(lineHeight);
}

void GMControlTextArea::placeCaret(GMint cp, bool adjustVisibleCP)
{
	D(d);
	D_BASE(db, Base);
	GM_ASSERT(cp >= 0);
	if (cp > d->buffer->getLength())
		cp = d->buffer->getLength();

	GMint firstX, firstY, x, y, x2, y2;
	d->buffer->CPtoXY(db->firstVisibleCP, false, &firstX, &firstY);
	d->buffer->CPtoXY(cp, false, &x, &y);

	if (cp == d->buffer->getLength())
	{
		x2 = x;
		y2 = y;
	}
	else
	{
		d->buffer->CPtoXY(cp, true, &x2, &y2); //trail
	}

	if (adjustVisibleCP)
	{
		if (y < firstY)
		{
			// 此时字符所在的位置比可见位置小
			db->firstVisibleCP = d->buffer->findFirstCPInOneLine(cp);
		}
		else if (y2 > firstY + db->rcText.height)
		{
			// 向下翻页的情况
			GMint yNewTop = y2 - db->rcText.height;
			GMint cpNew;
			d->buffer->XYtoCP(x, yNewTop, &cpNew);

			auto cpDiff = d->buffer->CPToLineNumber(db->cp) - d->buffer->CPToLineNumber(cp);
			GMint xNewFirst, yNewFirst;
			d->buffer->CPtoXY(cpNew, false, &xNewFirst, &yNewFirst);

			if (yNewFirst < yNewTop)
				++cpDiff;

			for (GMint i = 0; i < cpDiff; ++i)
			{
				db->firstVisibleCP = d->buffer->findLastCPInOneLine(db->firstVisibleCP) + 1;
			}

			//重置滚动偏移，重新计算firstX, firstY
			d->scrollOffset = 0;
			d->buffer->CPtoXY(db->firstVisibleCP, false, &firstX, &firstY);
		}
	}

	if (d->scrollOffset == 0)
	{
		// 在没有滚动偏移的情况下，光标越界了，需要偏移一个负值
		auto scrollOffset = (firstY + db->rcText.height) - (y + d->buffer->getLineHeight() + d->buffer->getLineSpacing());
		if (scrollOffset < 0)
			d->scrollOffset = scrollOffset;
	}
	else
	{
		GM_ASSERT(d->scrollOffset < 0);
		if (y - firstY + d->scrollOffset < 0)
			d->scrollOffset = 0;
	}

	db->cp = cp;
}

bool GMControlTextArea::handleMouse(GMSystemMouseEvent* event)
{
	D(d);
	if (hasScrollBarAndPointInScrollBarRect(event->getPoint()))
	{
		d->scrollBar->handleMouse(event);
	}
	return Base::handleMouse(event);
}

bool GMControlTextArea::onMouseMove(GMSystemMouseEvent* event)
{
	D(d);
	D_BASE(db, Base);
	if (d->hasScrollBar && d->scrollBar)
	{
		const GMPoint& pt = event->getPoint();
		if (hasScrollBarAndPointInScrollBarRect(pt))
		{
			GMWidget* widget = getParent();
			if (widget)
				widget->getParentWindow()->setCursor(GMCursorType::Arrow);
		}
		else if (GM_inRect(db->rcText, pt))
		{
			GMWidget* widget = getParent();
			if (widget)
				widget->getParentWindow()->setCursor(GMCursorType::IBeam);
		}
	}
	return Base::onMouseMove(event);
}

bool GMControlTextArea::onMouseDown(GMSystemMouseEvent* event)
{
	D(d);
	D_BASE(db, Base);
	if (!hasFocus())
		getParent()->requestFocus(this);

	const GMPoint& pt = event->getPoint();
	if (!containsPoint(pt))
		return false;

	if (GM_inRect(db->rcText, pt))
	{
		if (event->getModifier() & GMModifier_Shift)
			handleMouseSelect(event, false);
		else
			handleMouseSelect(event, true);
	}

	return true;
}

void GMControlTextArea::onFocusIn()
{
	D(d);
	Base::onFocusIn();
	if (d->hasScrollBar && d->scrollBar)
		d->scrollBar->onFocusIn();
}

void GMControlTextArea::onFocusOut()
{
	D(d);
	Base::onFocusOut();
	if (d->hasScrollBar && d->scrollBar)
		d->scrollBar->onFocusOut();
}

void GMControlTextArea::onMouseEnter()
{
	D(d);
	Base::onMouseEnter();
	if (d->hasScrollBar && d->scrollBar)
		d->scrollBar->onMouseEnter();
}

void GMControlTextArea::onMouseLeave()
{
	D(d);
	Base::onMouseLeave();
	if (d->hasScrollBar && d->scrollBar)
		d->scrollBar->onMouseLeave();
}

bool GMControlTextArea::onMouseWheel(GMSystemMouseWheelEvent* event)
{
	D(d);
	GMint detents = event->getDelta() / GM_WHEEL_DELTA;
	moveToLine(getCurrentVisibleLineNo() - detents);
	return true;
}

void GMControlTextArea::adjustInsertModeRect(REF GMRect& caretRc, GMint caretX)
{
	D(d);
	D_BASE(db, Base);
	GMint rightEdgeX;
	d->buffer->CPtoXY(db->cp, true, &rightEdgeX, nullptr);
	caretRc.width = rightEdgeX - caretX;
	if (caretRc.width <= 0)
		caretRc.width = 8; //给一个最小的光标宽度
	caretRc.height = 2;
	caretRc.y = getCaretTop() + getCaretHeight() - 2;
}

void GMControlTextArea::moveFirstVisibleCp(GMint distance)
{
	D(d);
	D_BASE(db, Base);
	if (db->firstVisibleCP > distance)
	{
		db->firstVisibleCP -= distance;
		db->firstVisibleCP = d->buffer->findFirstCPInOneLine(db->firstVisibleCP);
	}
	else
	{
		db->firstVisibleCP = 0;
	}
}

bool GMControlTextArea::onKey_UpDown(GMSystemKeyEvent* event)
{
	D(d);
	D_BASE(db, Base);
	GMKey key = event->getKey();
	GMint cp = 0;
	GMint x = 0, y = 0;
	d->buffer->CPtoMidXY(db->cp, &x, &y);
	GMint lh = d->buffer->getLineHeight() + d->buffer->getLineSpacing();
	if (event->getModifier() & GMModifier_Ctrl)
	{
		if (key == GMKey_Up)
			moveToLine(getCurrentVisibleLineNo() - 1);
		else if (key == GMKey_Down)
			moveToLine(getCurrentVisibleLineNo() + 1);
	}
	else
	{
		if (key == GMKey_Up)
		{
			if (y - lh > 0)
			{
				d->buffer->XYtoCP(x, y - lh, &cp);
				// 对于换行符，由于宽度为0，上一行对应的CP计算出来会少一个字符（因为是闭区间计算位置），所以要加回来
				if (d->buffer->isNewLine(db->cp) && x > 0)
					placeCaret(Min(cp + 1, d->buffer->findLastCPInOneLine(cp)), true);
				else
					placeCaret(cp, true);

				if (!(event->getModifier() & GMModifier_Shift))
					db->selectionStartCP = db->cp;
			}
		}
		else if (key == GMKey_Down)
		{
			d->buffer->XYtoCP(x, y + lh + d->buffer->getLineSpacing(), &cp);
			if (cp != d->buffer->findLastCPInOneLine(db->cp))
			{
				// 对于换行符，由于宽度为0，上一行对应的CP计算出来会少一个字符（因为是闭区间计算位置），所以要加回来
				if (d->buffer->isNewLine(db->cp) && x > 0)
					placeCaret(Min(cp + 1, d->buffer->findLastCPInOneLine(cp)), true);
				else
					placeCaret(cp, true);
			}

			if (!(event->getModifier() & GMModifier_Shift))
				db->selectionStartCP = db->cp;
		}
	}
	return true;
}

bool GMControlTextArea::onKey_HomeEnd(GMSystemKeyEvent* event)
{
	D(d);
	D_BASE(db, Base);
	GMKey key = event->getKey();
	if (key == GMKey_Home)
	{
		placeCaret(d->buffer->findFirstCPInOneLine(db->cp), true);
	}
	else
	{
		GM_ASSERT(key == GMKey_End);
		placeCaret(d->buffer->findLastCPInOneLine(db->cp), true);
	}

	if (!(event->getModifier() & GMModifier_Shift))
	{
		// 如果没有按住Shift，更新选区范围
		db->selectionStartCP = db->cp;
	}
	resetCaretBlink();
	return true;
}

bool GMControlTextArea::onKey_Back(GMSystemKeyEvent* event)
{
	D_BASE(d, Base);
	if (d->selectionStartCP != d->cp)
	{
		deleteSelectionText();
		emit(textChanged);
	}
	else if (d->cp > 0)
	{
		placeCaret(d->cp - 1, true);
		d->selectionStartCP = d->cp;
		if (d->buffer->removeChar(d->cp))
		{
			d->buffer->analyze(d->cp);
			emit(textChanged);
		}

		resetCaretBlink();
	}
	return true;
}

void GMControlTextArea::setBufferRenderRange(GMint xFirst, GMint yFirst)
{
	D(d);
	D_BASE(db, Base);
	// 获取能够显示的文本长度
	GMint lastCP;
	d->buffer->XYtoCP(xFirst + db->rcText.width, yFirst + db->rcText.height, &lastCP);
	GM_ASSERT(d->buffer->findFirstCPInOneLine(db->firstVisibleCP) == db->firstVisibleCP);
	d->buffer->setRenderRange(db->firstVisibleCP, lastCP);

	// 同时设置滚动条(如果有的话)
	if (d->hasScrollBar && d->scrollBar)
	{
		if (!isScrollBarLocked())
		{
			// 获取能显示多最大行数，并设置页步长
			d->scrollBar->setMinimum(1);
			d->scrollBar->setMaximum(d->buffer->CPToLineNumber(d->buffer->getLength()));
			d->scrollBar->setValue(d->buffer->CPToLineNumber(db->firstVisibleCP));
		}
	}
}

void GMControlTextArea::updateScrollBar()
{
	D(d);
	D_BASE(db, Base);
	if (d->hasScrollBar)
	{
		GMWidget* widget = getParent();
		if (!d->scrollBar)
		{
			d->scrollBar = gm_makeOwnedPtr<GMControlScrollBar>(widget);
			d->scrollBar->setPosition(db->rcText.x + db->rcText.width - d->scrollBarSize, db->rcText.y - 1);
			d->scrollBar->setSize(d->scrollBarSize, db->rcText.height + 2);
			d->scrollBar->setIsDefault(false);
			d->scrollBar->setCanRequestFocus(false);
			updateScrollBarPageStep();
			connect(*d->scrollBar, GMControlScrollBar::valueChanged, [](auto sender, auto receiver) {
				gm_cast<GMControlTextArea*>(receiver)->onScrollBarValueChanged(gm_cast<GMControlScrollBar*>(sender));
			});
			connect(*d->scrollBar, GMControlScrollBar::startDragThumb, [](auto sender, auto receiver) {
				gm_cast<GMControlTextArea*>(receiver)->lockScrollBar();
			});
			connect(*d->scrollBar, GMControlScrollBar::endDragThumb, [](auto sender, auto receiver) {
				gm_cast<GMControlTextArea*>(receiver)->unlockScrollBar();
			});
		}
	}
	updateRect();
}

void GMControlTextArea::updateScrollBarPageStep()
{
	D(d);
	D_BASE(db, Base);
	if (d->hasScrollBar && d->scrollBar)
	{
		auto lines = Round(db->rcText.height / (d->buffer->getLineHeight() + d->buffer->getLineSpacing()));
		d->scrollBar->setPageStep(lines - 1);
	}
}

bool GMControlTextArea::hasScrollBarAndPointInScrollBarRect(const GMPoint& pt)
{
	D(d);
	if (!d->hasScrollBar || !d->scrollBar)
		return false;

	return GM_inRect(d->scrollBar->getBoundingRect(), pt);
}

void GMControlTextArea::moveToLine(GMint lineNo)
{
	D(d);
	D_BASE(db, Base);
	GMint visibleLineNo = d->buffer->CPToLineNumber(db->firstVisibleCP);
	// 从当前的行数来遍历
	if (lineNo < visibleLineNo)
	{
		// 向上翻
		for (GMint i = db->firstVisibleCP; i >= 0; --i)
		{
			if (d->buffer->CPToLineNumber(i) == lineNo)
			{
				db->firstVisibleCP = d->buffer->findFirstCPInOneLine(i);
				break;
			}
		}
	}
	else if (lineNo > visibleLineNo)
	{
		// 向下翻
		for (GMint i = db->firstVisibleCP; i <= d->buffer->getLength(); ++i)
		{
			if (d->buffer->CPToLineNumber(i) == lineNo)
			{
				db->firstVisibleCP = i;
				break;
			}
		}
	}
}

GMint GMControlTextArea::getCurrentVisibleLineNo()
{
	D(d);
	D_BASE(db, Base);
	return d->buffer->CPToLineNumber(db->firstVisibleCP);
}

void GMControlTextArea::onScrollBarValueChanged(const GMControlScrollBar* sb)
{
	moveToLine(sb->getValue());
}

void GMControlTextArea::updateRect()
{
	Base::updateRect();

	D(d);
	D_BASE(db, Base);
	if (d->hasScrollBar)
	{
		// 如果拥有滚动条，渲染区域要缩小
		db->rcText.width -= d->scrollBarSize;
		d->buffer->setSize(db->rcText);
		updateScrollBarPageStep();
	}
}