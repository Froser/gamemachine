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
	BYTE keystate[256]{ 0 };
	bool keytriggerState[256]{ 0 };

	bool keydown(GMuint key)
	{
		return !! (keystate[key] & 0x80);
	}

	// 表示一个键是否按下一次，长按只算是一次
	bool keyTriggered(GMuint key)
	{
		return !! keytriggerState[key];
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
	enum { MAX_KEYS = 256 };
	bool mouseReady;
	bool mouseEnabled;
	GMUIWindow* window;
	XInputWrapper xinput;

	// 记录上一帧的按键
	BYTE keytriggerState[MAX_KEYS]{ 0 };
};

class Input_Windows : public GMObject
{
	DECLARE_PRIVATE(Input_Windows)

public:
	Input_Windows();
	~Input_Windows();

public:
	// 每一帧，应该调用一次update
	void update();
	void initMouse(GMUIWindow* window);
	void setMouseEnable(bool center);
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