#ifndef __GMCONTROLTEXTEDIT_H__
#define __GMCONTROLTEXTEDIT_H__
#include <gmcommon.h>
#include <gmcontrols.h>
#include <gmtransaction.h>

BEGIN_NS

class GMTypoTextBuffer;
class GMMultiLineTypoTextBuffer;

GM_PRIVATE_CLASS(GMControlTextEdit);
class GM_EXPORT GMControlTextEdit : public GMControl
{
	GM_DECLARE_PRIVATE(GMControlTextEdit)
	GM_DECLARE_BASE(GMControl)

public:
	enum StyleType
	{
		TextStyle = GMControl::StyleBegin,
	};

public:
	GM_DECLARE_SIGNAL(textChanged);

public:
	static GMControlTextEdit* createControl(
		GMWidget* widget,
		const GMString& text,
		GMint32 x,
		GMint32 y,
		GMint32 width,
		GMint32 height,
		bool isDefault
	);

protected:
	GMControlTextEdit(GMWidget* widget);

public:
	~GMControlTextEdit();

public:
	virtual void render(GMDuration elapsed) override;
	virtual void setSize(GMint32 width, GMint32 height) override;
	virtual void setPosition(GMint32 x, GMint32 y) override;
	virtual GMStyle& getStyle(GMControl::StyleType style) override;
	virtual void onFocusOut() override;
	virtual bool onKeyDown(GMSystemKeyEvent* event) override;
	virtual bool onMouseDown(GMSystemMouseEvent* event) override;
	virtual bool onMouseDblClick(GMSystemMouseEvent* event) override;
	virtual bool onMouseUp(GMSystemMouseEvent* event) override;
	virtual bool onMouseMove(GMSystemMouseEvent* event) override;
	virtual bool onChar(GMSystemCharEvent* event) override;
	virtual void onMouseEnter() override;
	virtual void onMouseLeave() override;
	virtual bool canHaveFocus() override;
	virtual void setText(const GMString& text);
	virtual void setPadding(GMint32 x, GMint32 y);
	virtual void placeCaret(GMint32 cP, bool adjustVisibleCP = true);
	virtual void placeSelectionStart(GMint32 selectionStartCP);

// 处理特殊按键
protected:
	virtual bool onKey_Tab(GMSystemKeyEvent* event);
	virtual bool onKey_LeftRight(GMSystemKeyEvent* event);
	virtual bool onKey_UpDown(GMSystemKeyEvent* event);
	virtual bool onKey_HomeEnd(GMSystemKeyEvent* event);
	virtual bool onKey_Delete(GMSystemKeyEvent* event);
	virtual bool onKey_Back(GMSystemKeyEvent* event);
	virtual bool onKey_Insert(GMSystemKeyEvent* event);

public:
	const GMString& getText() GM_NOEXCEPT;
	bool canUndo() GM_NOEXCEPT;
	void undo();
	bool canRedo() GM_NOEXCEPT;
	void redo();
	GMControlBorder* getBorder() GM_NOEXCEPT;
	void setBorderWidth(GMint32 width) GM_NOEXCEPT;
	void setShowCaret(bool showCaret) GM_NOEXCEPT;
	void setCaretBlinkSpeed(GMfloat blinkSpeedSecond) GM_NOEXCEPT;
	GMRect expandStencilRect(const GMRect& rc);

protected:
	virtual void renderCaret(GMint32 firstX, GMint32 caretX);
	virtual void pasteFromClipboard();
	virtual GMint32 getCaretTop();
	virtual GMint32 getCaretHeight();
	virtual void handleMouseCaret(const GMPoint& pt, bool selectStart);
	virtual void adjustInsertModeRect(REF GMRect& caretRc, GMint32 caretX);
	virtual void moveFirstVisibleCp(GMint32 distance);

protected:
	void createBufferTypoEngineIfNotExist();
	void blinkCaret(GMint32 firstX, GMint32 caretX);
	void moveCaret(bool next, bool newItem, bool select);
	bool deleteSelectionText();
	void selectAll();
	void resetCaretBlink();
	void copyToClipboard();
	void handleMouseSelect(GMSystemMouseEvent* event, bool selectStart);
	void initStyles(GMWidget* widget);
	void setBufferRenderRange(GMint32 xFirst);

protected:
	virtual void updateRect() override;
	virtual void insertCharacter(GMwchar ch);
};

GM_PRIVATE_CLASS(GMControlTextArea);
class GM_EXPORT GMControlTextArea : public GMControlTextEdit
{
	GM_DECLARE_PRIVATE(GMControlTextArea)
	GM_DECLARE_BASE(GMControlTextEdit)
	GM_DECLARE_PROPERTY(GMint32, ScrollBarSize)

public:
	static GMControlTextArea* createControl(
		GMWidget* widget,
		const GMString& text,
		GMint32 x,
		GMint32 y,
		GMint32 width,
		GMint32 height,
		bool isDefault,
		bool hasScrollBar
	);

	~GMControlTextArea();

protected:
	GMControlTextArea(GMWidget* widget);

public:
	virtual void render(GMDuration elapsed) override;
	virtual void pasteFromClipboard() override;
	virtual void insertCharacter(GMwchar ch) override;
	virtual void setSize(GMint32 width, GMint32 height) override;
	virtual GMint32 getCaretTop() override;
	virtual GMint32 getCaretHeight() override;
	virtual void handleMouseCaret(const GMPoint& pt, bool selectStart) override;
	virtual void placeCaret(GMint32 cp, bool adjustVisibleCP) override;
	virtual void updateRect() override;
	virtual bool handleMouse(GMSystemMouseEvent* event) override;
	virtual bool onMouseMove(GMSystemMouseEvent* event) override;
	virtual bool onMouseDown(GMSystemMouseEvent* event) override;
	virtual void onFocusIn() override;
	virtual void onFocusOut() override;
	virtual void onMouseEnter() override;
	virtual void onMouseLeave() override;
	virtual bool onMouseWheel(GMSystemMouseWheelEvent* event) override;
	virtual void adjustInsertModeRect(REF GMRect& caretRc, GMint32 caretX);
	virtual void moveFirstVisibleCp(GMint32 distance) override;

// 处理按键
	virtual bool onKey_UpDown(GMSystemKeyEvent* event) override;
	virtual bool onKey_HomeEnd(GMSystemKeyEvent* event) override;
	virtual bool onKey_Back(GMSystemKeyEvent* event) override;

public:
	void setScrollBar(bool scrollBar);
	void setLineSpacing(GMint32 lineSpacing) GM_NOEXCEPT;
	void setLineHeight(GMint32 lineHeight) GM_NOEXCEPT;
	GMControlScrollBar* getScrollBar() GM_NOEXCEPT;

protected:
	void setBufferRenderRange(GMint32 xFirst, GMint32 yFirst);

private:
	void updateScrollBar();
	void updateScrollBarPageStep();
	bool hasScrollBarAndPointInScrollBarRect(const GMPoint& pt);
	void moveToLine(GMint32 lineNo);
	GMint32 getCurrentVisibleLineNo();
	void onScrollBarValueChanged(const GMControlScrollBar* sb);
	inline void setCaretTopRelative(GMint32 caretTopRelative) GM_NOEXCEPT;
	inline GMRect adjustRectByScrollOffset(const GMRect& rc) GM_NOEXCEPT;
	void lockScrollBar() GM_NOEXCEPT;
	void unlockScrollBar() GM_NOEXCEPT;
	bool isScrollBarLocked() GM_NOEXCEPT;
};

END_NS
#endif