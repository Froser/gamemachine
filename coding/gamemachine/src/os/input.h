#ifndef __INPUT_H__
#define __INPUT_H__
#include "common.h"
#include <Xinput.h>
BEGIN_NS

struct JoystickState
{
	bool valid;
	WORD buttons;
	BYTE leftTrigger;
	BYTE rightTrigger;
	SHORT thumbLX;
	SHORT thumbLY;
	SHORT thumbRX;
	SHORT thumbRY;
};

struct KeyboardState
{
	BYTE keystate[256];

	GMint operator [](BYTE key)
	{
		return keystate[key] & 0x80;
	}
};

struct MouseState
{
	GMint deltaX;
	GMint deltaY;
};

struct IWindow;

class XInputWrapper
{
	typedef DWORD( WINAPI *XInputGetState_Delegate)(DWORD dwUserIndex, XINPUT_STATE* pState);
	typedef DWORD( WINAPI *XInputSetState_Delegate)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

public:
	XInputWrapper();
	~XInputWrapper();

public:
	DWORD XInputGetState(DWORD dwUserIndex, XINPUT_STATE* pState);
	DWORD XInputSetState(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

private:
	HMODULE m_module;
	XInputGetState_Delegate m_xinputGetState;
	XInputSetState_Delegate m_xinputSetState;
};

#ifdef _WINDOWS
class Input_Windows
{
public:
	Input_Windows();
	~Input_Windows();

public:
	void initMouse(IWindow* window);

public:
	JoystickState getJoystickState();
	void joystickVibrate(WORD leftMotorSpeed, WORD rightMotorSpeed);

	KeyboardState getKeyboardState();
	MouseState getMouseState();

private:
	bool m_mouseReady;
	IWindow* m_window;
	XInputWrapper m_xinput;
};

typedef Input_Windows Input;
#endif

END_NS
#endif