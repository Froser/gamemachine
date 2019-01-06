#ifndef __GMINPUT_H__
#define __GMINPUT_H__
#include <gmmessage.h>

#if _MSC_VER
#	include <Xinput.h>
#else
#	include <xinput_gm.h>
#endif

BEGIN_NS
struct IWindow;
struct GMJoystickState
{
	bool valid;
	GMWord buttons;
	GMbyte leftTrigger;
	GMbyte rightTrigger;
	GMshort thumbLX;
	GMshort thumbLY;
	GMshort thumbRX;
	GMshort thumbRY;
};

struct GMWheelState
{
	bool wheeled = false;
	GMshort delta = 0;
};

struct GMMouseState
{
	GMint32 deltaX;
	GMint32 deltaY;
	GMint32 posX;
	GMint32 posY;
	GMMouseButton downButton;
	GMMouseButton upButton;
	GMMouseButton triggerButton;
	bool wheeled;
	GMint32 wheeledDelta;
	bool moving;
};

GM_INTERFACE(IJoystickState)
{
	virtual void joystickVibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) = 0;
	virtual GMJoystickState joystickState() = 0;
};

GM_INTERFACE(IKeyboardState)
{
	virtual bool keydown(GMKey key) = 0;
	virtual bool keyTriggered(GMKey key) = 0;
};

GM_INTERFACE(IMouseState)
{
	virtual GMMouseState mouseState() = 0;
	virtual void setDetectingMode(bool enable) = 0;
};

GM_INTERFACE(IInput)
{
	virtual void update() = 0;
	virtual IKeyboardState& getKeyboardState() = 0;
	virtual IJoystickState& getJoystickState() = 0;
	virtual IMouseState& getMouseState() = 0;
	virtual void handleSystemEvent(GMSystemEvent* event) = 0;
};
END_NS
#endif