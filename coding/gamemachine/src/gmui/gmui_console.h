#ifndef __GMUI_CONSOLE_H__
#define __GMUI_CONSOLE_H__
#include "common.h"
#include "gmui.h"
BEGIN_NS

class GMUIConsole : public GMUIGUIWindow
{
public:
	virtual GMUIStringPtr getWindowClassName() const override;
};

END_NS
#endif