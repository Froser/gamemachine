#include "stdafx.h"
#include "gminput.h"

#ifdef _WINDOWS

static char* xinputDlls[] = {
	"xinput9_1_0.dll",
	"xinput1_4.dll",
	"xinput1_3.dll",
};

XInputWrapper::XInputWrapper()
	: m_xinputGetState(nullptr)
	, m_xinputSetState(nullptr)
	, m_module(0)
{
#ifdef _WINDOWS
	for (GMint i = 0; i < 3; i++)
	{
		m_module = LoadLibrary(xinputDlls[i]);
		if (m_module)
			break;
	}

	if (!m_module)
	{
		gm_warning("cannot find xinput dll, xinput disabled.");
		FreeLibrary(m_module);
		return;
	}

	m_xinputGetState = (XInputGetState_Delegate)GetProcAddress(m_module, "XInputGetState");
	m_xinputSetState = (XInputSetState_Delegate)GetProcAddress(m_module, "XInputSetState");
#endif
}

DWORD XInputWrapper::XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState)
{
	if (m_xinputGetState)
		return m_xinputGetState(dwUserIndex, pState);
	else
		gm_warning("cannot invoke XInputGetState");
	return ERROR_DLL_INIT_FAILED;
}

DWORD XInputWrapper::XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration)
{
	if (m_xinputSetState)
		return m_xinputSetState(dwUserIndex, pVibration);
	else
		gm_warning("cannot invoke XInputGetState");
	return ERROR_DLL_INIT_FAILED;
}

XInputWrapper::~XInputWrapper()
{
	FreeLibrary(m_module);
}

Input_Windows::Input_Windows()
{
	D(d);
	d->mouseReady = false;
}

Input_Windows::~Input_Windows()
{
}

void Input_Windows::initMouse(IWindow* window)
{
	D(d);
	d->window = window;
	GMRect rect = d->window->getWindowRect();
	::SetCursorPos(rect.x + rect.width / 2, rect.y + rect.height / 2);
	::ShowCursor(false);
	d->mouseReady = true;
}

GMJoystickState Input_Windows::getJoystickState()
{
	D(d);
	XINPUT_STATE state;
	GMJoystickState result = { false };

	if (d->xinput.XInputGetState(0, &state) == ERROR_SUCCESS)
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
	D(d);
	XINPUT_VIBRATION v = { leftMotorSpeed, rightMotorSpeed };
	d->xinput.XInputSetState(0, &v);
}

GMKeyboardState Input_Windows::getKeyboardState()
{
	GMKeyboardState state;
	GetKeyboardState(state.keystate);
	return std::move(state);
}

GMMouseState Input_Windows::getMouseState()
{
	D(d);
	if (!d->mouseReady)
	{
		gm_error("Mouse is not ready. Please call initMouse() first.");
		return GMMouseState();
	}

	GMMouseState state;
	GMRect rect = d->window->getWindowRect();
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