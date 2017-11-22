#ifndef __GMINPUT_WIN_H__
#define __GMINPUT_WIN_H__
#include <gmuicommon.h>
#include "gmuidef.h"
#include <gamemachine.h>

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

GM_PRIVATE_OBJECT(GMInput)
{
	enum { MAX_KEYS = 256 };
	bool mouseReady = false;
	bool detectingMode = false;
	gm::IWindow* window;

	// joystick (xinput)
	XInputWrapper xinput;
	gm::GMJoystickState joystickState;

	// keyboard
	gm::GMbyte keyState[256];
	gm::GMbyte lastKeyState[MAX_KEYS];

	// mouse
	gm::GMMouseState mouseState;
};

class GMInput :
	public gm::GMObject,
	public gm::IInput,
	public gm::IKeyboardState,
	public gm::IJoystickState,
	public gm::IMouseState
{
	DECLARE_PRIVATE(GMInput)

public:
	GMInput(gm::IWindow* window);

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
	virtual bool keydown(gm::GMuint key) override
	{
		D(d);
		return !!(d->keyState[key] & 0x80);
	}

	// 表示一个键是否按下一次，长按只算是一次
	virtual bool keyTriggered(gm::GMuint key) override
	{
		D(d);
		return !(d->lastKeyState[key] & 0x80) && (keydown(key));
	}

	// IJoystickState
public:
	virtual void joystickVibrate(gm::GMushort leftMotorSpeed, gm::GMushort rightMotorSpeed) override;
	virtual gm::GMJoystickState joystickState() override;
	virtual void setIMEState(bool enabled) override;

public:
	virtual gm::GMMouseState mouseState() override;
	virtual void setDetectingMode(bool center) override;
	virtual void setCursor(gm::GMCursorType type) override;
};
#endif