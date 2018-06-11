#ifndef __GMCONTROLTEXTEDIT_H__
#define __GMCONTROLTEXTEDIT_H__
#include <gmcommon.h>
#include <gmcontrols.h>
BEGIN_NS

class GMUniBuffer;

GM_PRIVATE_OBJECT(GMControlTextEdit)
{
	GMVec4 textColor = GMVec4(0, 0, 0, 1);
	GMVec4 selectionTextColor = GMVec4(0, 0, 0, 1);
	GMVec4 selectionBackColor = GMVec4(0, 0, 0, 1);
	GMVec4 caretColor = GMVec4(0, 0, 0, 1);
	GMint cp = 0;
	GMint firstVisibleCP = 0;
	GMint selectionStartCP = 0;
	GMUniBuffer* buffer = nullptr;
	GMRect rcText;
	GMStyle textStyle;
	GMControlBorder* borderControl = nullptr;
	GMint borderWidth = 5;
};

class GMControlTextEdit : public GMControl
{
	GM_DECLARE_PRIVATE(GMControlTextEdit)

public:
	GMControlTextEdit(GMWidget* widget);
	~GMControlTextEdit();

public:
	virtual void render(GMfloat elapsed) override;
	virtual void setSize(GMint width, GMint height) override;
	virtual void setPosition(GMint x, GMint y) override;
	virtual void setText(const GMString& text);

protected:
	void placeCaret(GMint cP);
	void deleteSelectionText();
	void resetCaretBlink();
	void copyToClipboard();
	void pasteFromClipboard();

public:
	inline GMControlBorder* getBorder()
	{
		D(d);
		return d->borderControl;
	}

	inline void setBorderWidth(GMint width)
	{
		D(d);
		d->borderWidth = width;
	}

protected:
	virtual void initStyles() override;
	virtual void updateRect() override;
};

END_NS
#endif