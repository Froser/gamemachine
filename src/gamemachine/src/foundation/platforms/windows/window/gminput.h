#ifndef __GMINPUT_WIN_H__
#define __GMINPUT_WIN_H__
#include <gmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include <gminput.h>

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
	bool detectingMode = false;
	IWindow* window;

	// joystick (xinput)
	XInputWrapper xinput;
	GMJoystickState joystickState;

	// keyboard
	GMbyte keyState[256];
	GMbyte lastKeyState[MAX_KEYS];

	// mouse
	GMMouseState mouseState;
	GMWheelState wheelState;
};

class GMInput :
	public GMObject,
	public IInput,
	public IKeyboardState,
	public IJoystickState,
	public IMouseState
{
	DECLARE_PRIVATE(GMInput)

public:
	GMInput(IWindow* window);

public:

	// IInput
public:
	// 每一帧，应该调用一次update
	virtual void update() override;
	virtual IKeyboardState& getKeyboardState() override;
	virtual IJoystickState& getJoystickState() override { return *this; }
	virtual IMouseState& getMouseState() override { return *this; }
	virtual void msgProc(GMSystemEvent* event) override;

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

public:
	virtual GMMouseState mouseState() override;
	virtual void setDetectingMode(bool center) override;
	virtual void setCursor(GMCursorType type) override;

private:
	void recordMouseDown(GMMouseButton button)
	{
		D(d);
		d->mouseState.downButton |= button;
	}

	void recordMouseUp(GMMouseButton button)
	{
		D(d);
		d->mouseState.upButton |= button;
	}

	void recordWheel(bool wheeled, GMshort delta)
	{
		D(d);
		d->wheelState.wheeled = wheeled;
		d->wheelState.delta = delta;
	}

	void recordMouseMove()
	{
		D(d);
		d->mouseState.moving = true;
	}
};
#endif