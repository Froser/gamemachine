#include "stdafx.h"
#include "input.h"
#include "gmengine/controllers/gamemachine.h"
#ifdef _WINDOWS

Input_Windows::Input_Windows()
	: m_mouseReady(false)
{
}

Input_Windows::~Input_Windows()
{
}

void Input_Windows::initMouse(IWindow* window)
{
	m_window = window;
	GMRect rect = m_window->getWindowRect();
	::SetCursorPos(rect.x + rect.width / 2, rect.y + rect.height / 2);
	::ShowCursor(false);
	m_mouseReady = true;
}

JoystickState Input_Windows::getJoystickState()
{
	XINPUT_STATE state;
	JoystickState result = { false };

	if (XInputGetState(0, &state) == ERROR_SUCCESS)
	{
		result.valid = true;
		result.leftTrigger = state.Gamepad.bLeftTrigger;
		result.rightTrigger = state.Gamepad.bRightTrigger;
		result.thumbLX = state.Gamepad.sThumbLX;
		result.thumbLY = state.Gamepad.sThumbLY;
		result.thumbRX = state.Gamepad.sThumbRX;
		result.thumbRY = state.Gamepad.sThumbRY;
		result.buttons = state.Gamepad.wButtons;
	}

	return result;
}

void Input_Windows::joystickVibrate(WORD leftMotorSpeed, WORD rightMotorSpeed)
{
	XINPUT_VIBRATION v = { leftMotorSpeed, rightMotorSpeed };
	XInputSetState(0, &v);
}

KeyboardState Input_Windows::getKeyboardState()
{
	KeyboardState state;
	GetKeyboardState(state.keystate);
	return std::move(state);
}

MouseState Input_Windows::getMouseState()
{
	if (!m_mouseReady)
	{
		gm_error("Mouse is not ready. Please call initMouse() first.");
		return MouseState();
	}

	MouseState state;
	GMRect rect = m_window->getWindowRect();
	const GMint centerX = rect.x + rect.width / 2;
	const GMint centerY = rect.y + rect.height / 2;

	POINT pos;
	::GetCursorPos(&pos);
	::SetCursorPos(centerX, centerY);
	state.deltaX = pos.x - centerX;
	state.deltaY = pos.y - centerY;
	return state;
}

#endif