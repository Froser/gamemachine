#ifndef __GMINPUT_WIN_H__
#define __GMINPUT_WIN_H__
#include <gmcommon.h>
#include <gminterfaces.h>
#include <gmobject.h>
#include <gminput.h>

BEGIN_NS

class JoystickStateImpl;
class MouseStateImpl;
class KeyboardStateImpl;

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

	// implements
	IJoystickState* joystickImpl = nullptr;
	IMouseState* mouseImpl = nullptr;
	IKeyboardState* keyboardImpl = nullptr;
	IIMState* imImpl = nullptr;
};

class GMInput :
	public GMObject,
	public IInput
{
	GM_DECLARE_PRIVATE(GMInput)

public:
	GMInput(IWindow* window);
	~GMInput();

public:

	// IInput
public:
	// 每一帧，应该调用一次update
	virtual void update() override;
	virtual IKeyboardState& getKeyboardState() override;
	virtual IJoystickState& getJoystickState() override;
	virtual IMouseState& getMouseState() override;
	virtual IIMState& getIMState() override;
	virtual void handleSystemEvent(GMSystemEvent* event) override;

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