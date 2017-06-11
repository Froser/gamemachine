#include "stdafx.h"
#include "gmui_console.h"

GMUIStringPtr GMUIConsole::getWindowClassName() const
{
	return _L("gamemachine_Console_class");
}

#if _WINDOWS
LongResult GMUIConsole::handleMessage(GMuint uMsg, UintPtr wParam, LongPtr lParam)
{
	if (uMsg == WM_CREATE)
	{

	}

	return GMUIWindow::handleMessage(uMsg, wParam, lParam);
}
#endif
