#ifndef __INPUT_H__
#define __INPUT_H__
#include "common.h"
BEGIN_NS

#if _WINDOWS
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
#else
struct GMJoystickState
{
};
#endif

struct GMMouseState
{
	GMint deltaX;
	GMint deltaY;
};

GM_INTERFACE(IJoystickState)
{
	virtual void joystickVibrate(GMshort leftMotorSpeed, GMshort rightMotorSpeed) = 0;
	virtual GMJoystickState joystickState() = 0;
};

GM_INTERFACE(IKeyboardState)
{
	virtual bool keydown(GMuint key) = 0;
	virtual bool keyTriggered(GMuint key) = 0;
};

GM_INTERFACE(IMouseState)
{
	virtual GMMouseState mouseState() = 0;
	virtual void initMouse(GMUIWindow* window) = 0;
	virtual void setMouseEnable(bool enable) = 0;
};

#if _WINDOWS
#include <Xinput.h>

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

	// joystick (xinput)
	XInputWrapper xinput;
	GMJoystickState joystickState;

	// keyboard
	BYTE keyState[256];
	BYTE lastKeyState[MAX_KEYS];

	// mouse
	GMMouseState mouseState;
};

class Input_Windows :
	public GMObject,
	public IKeyboardState,
	public IJoystickState,
	public IMouseState
{
	DECLARE_PRIVATE(Input_Windows)

public:
	Input_Windows();

public:
	// 每一帧，应该调用一次update
	void update();

	IKeyboardState& getKeyboardState();
	IJoystickState& getJoystickState() { return *this; }
	IMouseState& getMouseState() { return *this; }

	// IKeyboardState
public:
	virtual bool keydown(GMuint key) override
	{
		D(d);
		return !!(d->keyState[key] & 0x80);
	}

	// 表示一个键是否按下一次，长按只算是一次
	virtual bool keyTriggered(GMuint key) override
	{
		D(d);
		return !(d->lastKeyState[key] & 0x80) && (keydown(key));
	}

	// IJoystickState
public:
	virtual void joystickVibrate(GMshort leftMotorSpeed, GMshort rightMotorSpeed) override;
	virtual GMJoystickState joystickState() override;

	// IMouseState
public:
	virtual void initMouse(GMUIWindow* window) override;
	virtual GMMouseState mouseState() override;
	virtual void setMouseEnable(bool center) override;
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