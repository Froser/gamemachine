#ifndef __GMUI_CONSOLE_H__
#define __GMUI_CONSOLE_H__
#include "common.h"
#include "gmui.h"
BEGIN_NS

class GMUIConsole : public GMUIWindow
{
public:
	virtual GMUIStringPtr getWindowClassName() const override;
	virtual LongResult handleMessage(GMuint uMsg, UintPtr wParam, LongPtr lParam) override;
};

END_NS
#endif