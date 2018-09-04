#include "stdafx.h"
#include "gminput.h"
#include <gamemachine.h>

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
	//TODO
	return state;
}