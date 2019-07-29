#include "stdafx.h"
#include "gminput.h"
#include <gamemachine.h>
#include "gmxrendercontext.h"

BEGIN_NS
namespace
{
	void setCursorPos(Display* display, Window root, GMint32 x, GMint32 y)
	{
		XWarpPointer(display, None, root, 0, 0, 0, 0, x, y);
		XSync(display, False);
	}
}

class JoystickStateImpl : public IJoystickState
{
public:
	JoystickStateImpl(GMInput* host) { m_host = host; }
	virtual void vibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed) override;
	virtual GMJoystickState state() override;
private:
	GMInput* m_host = nullptr;
};

class MouseStateImpl : public IMouseState
{
public:
	MouseStateImpl(GMInput* host) { m_host = host; }
	virtual GMMouseState state() override;
	virtual void setDetectingMode(bool center) override;
private:
	GMInput* m_host = nullptr;
};

class KeyboardStateImpl : public IKeyboardState
{
public:
	KeyboardStateImpl(GMInput* host) { m_host = host; }
	virtual bool keydown(GMKey key) override;
	virtual bool keyTriggered(GMKey key) override;
private:
	GMInput* m_host = nullptr;
};

class IMStateImpl : public IIMState
{
public:
	virtual void activate(GMKeyboardLayout layout) { /*TODO*/ }
};


GM_PRIVATE_OBJECT_UNALIGNED(GMInput)
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

GMInput::GMInput(IWindow* window)
{
	GM_CREATE_DATA();
	D(d);
	d->window = window;
	d->joystickImpl = new JoystickStateImpl(this);
	d->mouseImpl = new MouseStateImpl(this);
	d->keyboardImpl = new KeyboardStateImpl(this);
	d->imImpl = new IMStateImpl();
}

GMInput::~GMInput()
{
	D(d);
	GM_delete(d->mouseImpl);
	GM_delete(d->joystickImpl);
	GM_delete(d->keyboardImpl);
	GM_delete(d->imImpl);
}

void GMInput::update()
{
	D(d);
	// restore
	d->wheelState.wheeled = false;
	d->wheelState.delta = 0;
	d->mouseState.downButton = d->mouseState.upButton = GMMouseButton_None;
	d->mouseState.moving = false;
}

IKeyboardState& GMInput::getKeyboardState()
{
	D(d);
	GM_ASSERT(d->keyboardImpl);
	return *d->keyboardImpl;
}

IJoystickState& GMInput::getJoystickState()
{
	D(d);
	GM_ASSERT(d->joystickImpl);
	return *d->joystickImpl;
}

IMouseState& GMInput::getMouseState()
{
	D(d);
	GM_ASSERT(d->mouseImpl);
	return *d->mouseImpl;
}

IIMState& GMInput::getIMState()
{
	D(d);
	GM_ASSERT(d->imImpl);
	return *d->imImpl;
}

void GMInput::handleSystemEvent(GMSystemEvent* event)
{
	GMSystemEventType type = event->getType();
	switch (type)
	{
	case GMSystemEventType::MouseWheel:
	{
		GMSystemMouseWheelEvent* e = static_cast<GMSystemMouseWheelEvent*>(event);
		recordWheel(true, e->getDelta());
		break;
	}
	case GMSystemEventType::MouseMove:
	case GMSystemEventType::MouseDown:
	case GMSystemEventType::MouseUp:
	{
		GMSystemMouseEvent* e = static_cast<GMSystemMouseEvent*>(event);
		if (type == GMSystemEventType::MouseMove)
		{
			recordMouseMove();
		}
		else if (type == GMSystemEventType::MouseDown)
		{
			recordMouseDown(e->getButton());
		}
		else
		{
			GM_ASSERT(type == GMSystemEventType::MouseUp);
			recordMouseUp(e->getButton());
		}
		break;
	}
	}
}

void GMInput::recordMouseDown(GMMouseButton button)
{
	D(d);
	d->mouseState.downButton |= button;
}

void GMInput::recordMouseUp(GMMouseButton button)
{
	D(d);
	d->mouseState.upButton |= button;
}

void GMInput::recordWheel(bool wheeled, GMshort delta)
{
	D(d);
	d->wheelState.wheeled = wheeled;
	d->wheelState.delta = delta;
}

void GMInput::recordMouseMove()
{
	D(d);
	d->mouseState.moving = true;
}
//////////////////////////////////////////////////////////////////////////
void MouseStateImpl::setDetectingMode(bool enable)
{
	D_OF(d, m_host);
	//TODO
	d->detectingMode = enable;
	const GMRect& rect = d->window->getWindowStates().renderRect;
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(d->window->getContext());
	Display* display = context->getDisplay();
	setCursorPos(display, context->getRootWindow(), rect.x + rect.width / 2, rect.y + rect.height / 2);
}

GMMouseState MouseStateImpl::state()
{
	D_OF(d, m_host);
	GMMouseState& state = d->mouseState;
	state.wheeled = d->wheelState.wheeled;
	state.wheeledDelta = static_cast<GMint32>(d->wheelState.delta);
	
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(d->window->getContext());
	Display* display = context->getDisplay();
	Window window = context->getWindow()->getWindowHandle();

	Window rw, cw;
	GMint32 rx, ry;
	GMint32 x, y;
	GMuint32 mask;
	if (XQueryPointer(
		display, 
		window, 
		&rw, &cw, //root, child
		&rx, &ry, //root x, y
		&x, &y,
		&mask
	))
	{
		state.posX = x;
		state.posY = y;

/*
		IKeyboardState& ks = getKeyboardState();
		if (ks.keyTriggered(GMKey_Lbutton))
			state.triggerButton |= GMMouseButton_Left;
		if (ks.keyTriggered(GMKey_Rbutton))
			state.triggerButton |= GMMouseButton_Right;
		if (ks.keyTriggered(GMKey_Mbutton))
			state.triggerButton |= GMMouseButton_Middle;
			*/

		if (d->detectingMode)
		{
			GMRect rect = d->window->getWindowRect();
			const GMint32 centerX = rect.x + rect.width / 2;
			const GMint32 centerY = rect.y + rect.height / 2;
			setCursorPos(display, context->getRootWindow(), centerX, centerY);
			state.deltaX = x - centerX;
			state.deltaY = y - centerY;
		}
		else
		{
			state.deltaX = state.deltaY = 0;
		}
	}

	return state;
}

GMJoystickState JoystickStateImpl::state()
{
	D_OF(d, m_host);
	//TODO
	GMJoystickState result = { false };
	return std::move(result);
}

void JoystickStateImpl::vibrate(GMushort leftMotorSpeed, GMushort rightMotorSpeed)
{
	//TODO
}

bool KeyboardStateImpl::keydown(GMKey key)
{
	//TODO
	return false;
}

// 表示一个键是否按下一次，长按只算是一次
bool KeyboardStateImpl::keyTriggered(GMKey key)
{
	return false;
}

END_NS