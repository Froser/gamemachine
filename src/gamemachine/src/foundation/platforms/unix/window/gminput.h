#ifndef __GMINPUT_WIN_H__
#define __GMINPUT_WIN_H__
#include <gmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include <gminput.h>

BEGIN_NS

GM_PRIVATE_OBJECT(GMInput)
{
	enum { MAX_KEYS = 256 };
	bool detectingMode = false;
	IWindow* window = nullptr;

	GMJoystickState joystickState;

	// keyboard
	GMbyte keyState[256] = {0};
	GMbyte lastKeyState[MAX_KEYS] = {0};

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
	GM_DECLARE_PRIVATE(GMInput)

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
	virtual void handleSystemEvent(GMSystemEvent* event) override;

	// IKeyboardState
public:
	//! 返回某个键是否此时被按下。
	/*!
	  \param key 待测试的键。
	  \return 返回是否此键被按下。
	*/
	virtual bool keydown(GMKey key) override;

	//! 表示一个键是否按下一次，长按只算是一次
	/*!
	  \param key 按下的键。
	  \return 返回是否此键被按下一次。
	*/
	virtual bool keyTriggered(GMKey key) override;

	// IJoystickState
public:
	virtual void joystickVibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) override;
	virtual GMJoystickState joystickState() override;

public:
	virtual GMMouseState mouseState() override;
	virtual void setDetectingMode(bool center) override;

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

END_NS

#endif