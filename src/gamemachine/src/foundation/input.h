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
	virtual void vibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) = 0;
	virtual GMJoystickState state() = 0;
};

GM_INTERFACE(IKeyboardState)
{
	//! 返回某个键是否此时被按下。
	/*!
	  \param key 待测试的键。
	  \return 返回是否此键被按下。
	*/
	virtual bool keydown(GMKey key) = 0;

	//! 表示一个键是否按下一次，长按只算是一次
	/*!
	  \param key 按下的键。
	  \return 返回是否此键被按下一次。
	*/
	virtual bool keyTriggered(GMKey key) = 0;
};

GM_INTERFACE(IMouseState)
{
	virtual void setDetectingMode(bool enable) = 0;
	virtual GMMouseState state() = 0;
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