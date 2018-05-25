#include "stdafx.h"
#include "check.h"
#include "defines.h"
#include "../../gamemachine.h"

// 这个方法，在不同系统下应该有不同实现。
void GameMachine::runEventLoop()
{
	MSG msg;
	msg.message = WM_NULL;
	PeekMessage(&msg, NULL, 0U, 0U, PM_NOREMOVE);

	BOOL bGotMessage = FALSE;
	while (WM_QUIT != msg.message)
	{
		bGotMessage = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);

		if (bGotMessage)
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			if (!renderFrame())
				break;
		}
	}
	terminate();
}

void GameMachine::translateSystemEvent(GMuint uMsg, GMWParam wParam, GMLParam lParam, GMLResult* lRes, OUT GMSystemEvent** event)
{
	GM_ASSERT(event);
	switch (uMsg)
	{
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_KEYUP:
	case WM_SYSKEYUP:
	{
		*event = new GMSystemKeyEvent();
	}
	}
}