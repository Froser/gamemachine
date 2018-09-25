#include "stdafx.h"
#include "gminput.h"
#include <gamemachine.h>
#include "gmxrendercontext.h"

namespace
{
	void setCursorPos(Display* display, Window root, GMint32 x, GMint32 y)
	{
		XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
		XSync(display, False);
	}
}

GMInput::GMInput(IWindow* window)
{
	D(d);
	d->window = window;
}

void GMInput::update()
{
	D(d);
	// restore
	d->wheelState.wheeled = false;
	d->mouseState.downButton = d->mouseState.upButton = GMMouseButton_None;
	d->mouseState.moving = false;
}

void GMInput::setDetectingMode(bool enable)
{
	D(d);
	//TODO
	d->detectingMode = enable;
	const GMRect& rect = d->window->getWindowStates().renderRect;
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(d->window->getContext());
	Display* display = context->getDisplay();
	setCursorPos(display, context->getRootWindow(), rect.x + rect.width / 2, rect.y + rect.height / 2);
}

GMJoystickState GMInput::joystickState()
{
	D(d);
	//TODO
	GMJoystickState result = { false };
	return std::move(result);
}

void GMInput::joystickVibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed)
{
	D(d);
	//TODO
}

IKeyboardState& GMInput::getKeyboardState()
{
	GM_PROFILE("getKeyboardState");
	D(d);
	//TODO
	return *this;
}

void GMInput::msgProc(GMSystemEvent* event)
{
	GMSystemEventType type = event->getType();
	switch (type)
	{
	case GMSystemEventType::MouseWheel:
	{
		GMSystemMouseWheelEvent* e = gm_cast<GMSystemMouseWheelEvent*>(event);
		recordWheel(true, e->getDelta());
		break;
	}
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
	{
		GMSystemMouseEvent* e = gm_cast<GMSystemMouseEvent*>(event);
		if (type == GMSystemEventType::MouseMove)
		{
			recordMouseMove();
		}
		else if (type == GMSystemEventType::MouseDown)
		{
			recordMouseDown(e->getButton());
		}
		else
		{
			GM_ASSERT(type == GMSystemEventType::MouseUp);
			recordMouseUp(e->getButton());
		}
		break;
	}
	}
}

bool GMInput::keydown(GMKey key)
{
	D(d);
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(d->window->getContext());


	//TODO
	return false;
}

// 表示一个键是否按下一次，长按只算是一次
bool GMInput::keyTriggered(GMKey key)
{
	D(d);
	//return !(d->lastKeyState[getWindowsKey(key)] & 0x80) && (keydown(key));
	return false;
}

GMMouseState GMInput::mouseState()
{
	D(d);
	GMMouseState& state = d->mouseState;
	state.wheel = d->wheelState;
	
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(d->window->getContext());
	Display* display = context->getDisplay();
	Window window = context->getWindow()->getWindowHandle();

	Window rw, cw;
	GMint32 rx, ry;
	GMint32 x, y;
	GMuint32 mask;
	if (XQueryPointer(
		display, 
		window, 
		&rw, &cw, //root, child
		&rx, &ry, //root x, y
		&x, &y,
		&mask
	))
	{
		state.posX = x;
		state.posY = y;

/*
		IKeyboardState& ks = getKeyboardState();
		if (ks.keyTriggered(GMKey_Lbutton))
			state.triggerButton |= GMMouseButton_Left;
		if (ks.keyTriggered(GMKey_Rbutton))
			state.triggerButton |= GMMouseButton_Right;
		if (ks.keyTriggered(GMKey_Mbutton))
			state.triggerButton |= GMMouseButton_Middle;
			*/

		if (d->detectingMode)
		{
			GMRect rect = d->window->getWindowRect();
			const GMint32 centerX = rect.x + rect.width / 2;
			const GMint32 centerY = rect.y + rect.height / 2;
			setCursorPos(display, context->getRootWindow(), centerX, centerY);
			state.deltaX = x - centerX;
			state.deltaY = y - centerY;
		}
		else
		{
			state.deltaX = state.deltaY = 0;
		}
	}

	return state;
}