#ifndef __INPUT_H__
#define __INPUT_H__
#include "common.h"
BEGIN_NS

#if _WINDOWS
#include <Xinput.h>

struct GMJoystickState
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

struct GMKeyboardState
{
	BYTE keystate[256];

	GMint operator [](BYTE key)
	{
		return keystate[key] & 0x80;
	}
};

struct GMMouseState
{
	GMint deltaX;
	GMint deltaY;
};

class GMUIWindow;
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

GM_PRIVATE_OBJECT(Input_Windows)
{
	bool mouseReady;
	GMUIWindow* window;
	XInputWrapper xinput;
};

class Input_Windows : public GMObject
{
	DECLARE_PRIVATE(Input_Windows)

public:
	Input_Windows();
	~Input_Windows();

public:
	void initMouse(GMUIWindow* window);
	GMJoystickState getJoystickState();
	void joystickVibrate(WORD leftMotorSpeed, WORD rightMotorSpeed);

	GMKeyboardState getKeyboardState();
	GMMouseState getMouseState();
};
#endif

struct InputSelector
{
#if _WINDOWS
	typedef Input_Windows InputTarget;
#endif
};

class GMInput : public InputSelector::InputTarget
{
	typedef InputSelector::InputTarget Base;
	friend class GameMachine;

private:
	GMInput() : Base() {}
};

END_NS
#endif