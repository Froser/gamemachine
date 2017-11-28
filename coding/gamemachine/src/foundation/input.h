#ifndef __GMINPUT_H__
#define __GMINPUT_H__
#include <gmcommon.h>

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
	WORD buttons;
	GMbyte leftTrigger;
	GMbyte rightTrigger;
	GMshort thumbLX;
	GMshort thumbLY;
	GMshort thumbRX;
	GMshort thumbRY;
};


// GMMouseButton defines:
#define GMMouseButton_None		0
#define GMMouseButton_Left		1
#define GMMouseButton_Right		2
#define GMMouseButton_Middle	4

struct GMWheelState
{
	bool wheeled = false;
	gm::GMshort delta = 0;
};

struct GMMouseState
{
	typedef GMint GMMouseButton;

	GMint deltaX;
	GMint deltaY;
	GMint posX;
	GMint posY;
	GMMouseButton down_button;
	GMMouseButton trigger_button;
	GMWheelState wheel;
};

enum class GMCursorType
{
	Arrow,
	IBeam,
	Wait,
	Cross,
	UpArrow,
	Hand,
	Custom,
};

GM_INTERFACE(IJoystickState)
{
	virtual void joystickVibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) = 0;
	virtual GMJoystickState joystickState() = 0;
};

GM_INTERFACE(IKeyboardState)
{
	virtual bool keydown(GMuint key) = 0;
	virtual bool keyTriggered(GMuint key) = 0;
	virtual void setIMEState(bool enabled) = 0;
};

GM_INTERFACE(IMouseState)
{
	virtual GMMouseState mouseState() = 0;
	virtual void setDetectingMode(bool enable) = 0;
	virtual void setCursor(GMCursorType type) = 0;
};

GM_INTERFACE(IInput)
{
	virtual void update() = 0;
	virtual IKeyboardState& getKeyboardState() = 0;
	virtual IJoystickState& getJoystickState() = 0;
	virtual IMouseState& getMouseState() = 0;
};
END_NS
#endif