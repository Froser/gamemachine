#ifndef __MOUSE_H__
#define __MOUSE_H__
#include "common.h"
#include "utilities/autoptr.h"
BEGIN_NS

struct Mouse
{
	static void getCursorPosition(int* x, int* y);
	static void setCursorPosition(int x, int y);
	static void showCursor(bool show);
};

struct IMouseReactionHandler
{
	virtual void onMouseMove(GMfloat deltaX, GMfloat deltaY) = 0;
};

class MouseReaction
{
public:
	MouseReaction(AUTORELEASE IMouseReactionHandler* handler);

public:
	void initReaction(GMRect rect);
	void mouseReact(GMRect rect);

private:
	AutoPtr<IMouseReactionHandler> m_handler;
};

END_NS
#endif