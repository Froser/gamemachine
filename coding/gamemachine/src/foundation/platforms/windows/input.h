#ifndef __GMINPUT_WIN_H__
#define __GMINPUT_WIN_H__
#include <gmcommon.h>
#include "check.h"
#include "io/gminput.h"

BEGIN_NS

class XInputWrapper
{
	typedef DWORD(WINAPI *XInputGetState_Delegate)(DWORD dwUserIndex, XINPUT_STATE* pState);
	typedef DWORD(WINAPI *XInputSetState_Delegate)(DWORD dwUserIndex, XINPUT_VIBRATION* pVibration);

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

GM_PRIVATE_OBJECT(GMInput_Windows)
{
	enum { MAX_KEYS = 256 };
	bool mouseReady = false;
	bool mouseEnabled = true;
	IWindow* window;

	// joystick (xinput)
	XInputWrapper xinput;
	GMJoystickState joystickState;

	// keyboard
	GMbyte keyState[256];
	GMbyte lastKeyState[MAX_KEYS];

	// mouse
	GMMouseState mouseState;
};

class GMInput_Windows :
	public GMObject,
	public IInput,
	public IKeyboardState,
	public IJoystickState,
	public IMouseState
{
	DECLARE_PRIVATE(GMInput_Windows)

public:
	GMInput_Windows() = default;

public:

	// IInput
public:
	// 每一帧，应该调用一次update
	virtual void update() override;
	virtual IKeyboardState& getKeyboardState() override;
	virtual IJoystickState& getJoystickState() override { return *this; }
	virtual IMouseState& getMouseState() override { return *this; }

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
	virtual void joystickVibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) override;
	virtual GMJoystickState joystickState() override;
	virtual void setIMEState(bool enabled) override;

	// IMouseState
public:
	virtual void initMouse(IWindow* window) override;
	virtual GMMouseState mouseState() override;
	virtual void setMouseEnable(bool center) override;
};

END_NS
#endif