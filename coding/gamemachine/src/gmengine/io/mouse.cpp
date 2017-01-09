#include "stdafx.h"
#include "mouse.h"
#ifdef _WINDOWS
#	include <windows.h>
#endif

void Mouse::getCursorPosition(int* x, int* y)
{
#ifdef _WINDOWS
	POINT pos;
	::GetCursorPos(&pos);
	*x = pos.x;
	*y = pos.y;
#else
	ASSERT(false);
#endif
}

void Mouse::setCursorPosition(int x, int y)
{
#ifdef _WINDOWS
	::SetCursorPos(x, y);
#else
	ASSERT(false);
#endif
}

void Mouse::showCursor(bool show)
{
#ifdef _WINDOWS
	::ShowCursor(show ? TRUE : FALSE);
#else
	ASSERT(false);
#endif
}


MouseReaction::MouseReaction(AUTORELEASE IMouseReactionHandler* handler)
	: m_handler(handler)
{

}

void MouseReaction::initReaction(GMRect rect)
{
	const int centerX = rect.x + rect.width / 2;
	const int centerY = rect.y + rect.height / 2;
	Mouse::setCursorPosition(centerX, centerY);
	Mouse::showCursor(false);
}

void MouseReaction::mouseReact(GMRect rect)
{
	const int centerX = rect.x + rect.width / 2;
	const int centerY = rect.y + rect.height/ 2;

	int x = 0, y = 0;
	Mouse::getCursorPosition(&x, &y);
	int deltaX = x - centerX, deltaY = y - centerY;
	m_handler->onMouseMove(deltaX, deltaY);
	Mouse::setCursorPosition(centerX, centerY);
}


