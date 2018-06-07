#ifndef __GMCONTROLTEXTEDIT_H__
#define __GMCONTROLTEXTEDIT_H__
#include <gmcommon.h>
#include <gmcontrols.h>
BEGIN_NS

GM_PRIVATE_OBJECT(GMControlTextEdit)
{
	GMVec4 textColor;
	GMVec4 selectionTextColor;
	GMVec4 selectionBackColor;
	GMVec4 caretColor;
};

class GMControlTextEdit : public GMControl
{
	GM_DECLARE_PRIVATE(GMControlTextEdit)

public:
	virtual void render(float elapsed) override;
};

END_NS
#endif