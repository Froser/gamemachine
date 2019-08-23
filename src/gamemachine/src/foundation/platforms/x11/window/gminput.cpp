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

	KeySym getKeysym(GMKey key)
	{
		static HashMap<GMKey, GMWParam> s_keyMap;
		static std::once_flag s_flag;
		std::call_once(s_flag, [](auto& keyMap) {
			keyMap[GMKey_Cancel] = XK_Cancel;
			keyMap[GMKey_Back] = XK_BackSpace;
			keyMap[GMKey_Tab] = XK_Tab;
			keyMap[GMKey_Clear] = XK_Clear;
			keyMap[GMKey_Return] = XK_Return;
			keyMap[GMKey_Shift] = XK_Shift_L;
			keyMap[GMKey_Control] = XK_Control_L;
			keyMap[GMKey_Menu] = XK_Menu;
			keyMap[GMKey_Pause] = XK_Pause;
			keyMap[GMKey_Capital] = XK_Caps_Lock;
			keyMap[GMKey_Kana] = XK_kana_A;
			keyMap[GMKey_Hangul] = XK_Hangul;
			keyMap[GMKey_Hanja] = XK_Hangul_Hanja;
			keyMap[GMKey_Kanji] = XK_Kanji;
			keyMap[GMKey_Escape] = XK_Escape;
			keyMap[GMKey_Modechange] = XK_Mode_switch;
			keyMap[GMKey_Space] = XK_space;
			keyMap[GMKey_Prior] = XK_Page_Up;
			keyMap[GMKey_Next] = XK_Page_Down;
			keyMap[GMKey_End] = XK_End;
			keyMap[GMKey_Home] = XK_Home;
			keyMap[GMKey_Left] = XK_Left;
			keyMap[GMKey_Up] = XK_Up;
			keyMap[GMKey_Right] = XK_Right;
			keyMap[GMKey_Down] = XK_Down;
			keyMap[GMKey_Select] = XK_Select;
			keyMap[GMKey_Print] = XK_Print;
			keyMap[GMKey_Execute] = XK_Execute;
			keyMap[GMKey_Snapshot] = XK_Print;
			keyMap[GMKey_Insert] = XK_Insert;
			keyMap[GMKey_Delete] = XK_Delete;
			keyMap[GMKey_Help] = XK_Help;
			keyMap[GMKey_Lwin] = XK_Super_L;
			keyMap[GMKey_Rwin] = XK_Super_R;
			keyMap[GMKey_Apps] = XK_Menu;
			keyMap[GMKey_Numpad0] = XK_KP_0;
			keyMap[GMKey_Numpad1] = XK_KP_1;
			keyMap[GMKey_Numpad2] = XK_KP_2;
			keyMap[GMKey_Numpad3] = XK_KP_3;
			keyMap[GMKey_Numpad4] = XK_KP_4;
			keyMap[GMKey_Numpad5] = XK_KP_5;
			keyMap[GMKey_Numpad6] = XK_KP_6;
			keyMap[GMKey_Numpad7] = XK_KP_7;
			keyMap[GMKey_Numpad8] = XK_KP_8;
			keyMap[GMKey_Numpad9] = XK_KP_9;
			keyMap[GMKey_Multiply] = XK_KP_Multiply;
			keyMap[GMKey_Add] = XK_KP_Add;
			keyMap[GMKey_Subtract] = XK_KP_Subtract;
			keyMap[GMKey_Decimal] = XK_KP_Decimal;
			keyMap[GMKey_Divide] = XK_KP_Divide;
			keyMap[GMKey_F1] = XK_F1;
			keyMap[GMKey_F2] = XK_F2;
			keyMap[GMKey_F3] = XK_F3;
			keyMap[GMKey_F4] = XK_F4;
			keyMap[GMKey_F5] = XK_F5;
			keyMap[GMKey_F6] = XK_F6;
			keyMap[GMKey_F7] = XK_F7;
			keyMap[GMKey_F8] = XK_F8;
			keyMap[GMKey_F9] = XK_F9;
			keyMap[GMKey_F10] = XK_F10;
			keyMap[GMKey_F11] = XK_F11;
			keyMap[GMKey_F12] = XK_F12;
			keyMap[GMKey_F13] = XK_F13;
			keyMap[GMKey_F14] = XK_F14;
			keyMap[GMKey_F15] = XK_F15;
			keyMap[GMKey_F16] = XK_F16;
			keyMap[GMKey_F17] = XK_F17;
			keyMap[GMKey_F18] = XK_F18;
			keyMap[GMKey_F19] = XK_F19;
			keyMap[GMKey_F20] = XK_F20;
			keyMap[GMKey_F21] = XK_F21;
			keyMap[GMKey_F22] = XK_F22;
			keyMap[GMKey_F23] = XK_F23;
			keyMap[GMKey_F24] = XK_F24;
			keyMap[GMKey_Numlock] = XK_Num_Lock;
			keyMap[GMKey_Scroll] = XK_Scroll_Lock;
			keyMap[GMKey_Lshift] = XK_Shift_L;
			keyMap[GMKey_Rshift] = XK_Shift_R;
			keyMap[GMKey_Lcontrol] = XK_Control_L;
			keyMap[GMKey_Rcontrol] = XK_Control_R;
			keyMap[GMKey_Lmenu] = XK_Menu;
			keyMap[GMKey_Rmenu] = XK_Menu;
		}, s_keyMap);

		auto iter = s_keyMap.find(key);
		if (iter != s_keyMap.end())
			return iter->second;

		return GM_KeyToASCII(key);
	}

	bool isKeyDown(Display* display, GMKey key, GMbyte* keys)
	{
		KeySym keysym = getKeysym(key);
		KeyCode kc = XKeysymToKeycode(display, keysym);
		return !!(keys[kc >> 3] & (1 << (kc & 7)));
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
	enum { MAX_KEYS = 32 };
	bool detectingMode = false;
	IWindow* window = nullptr;

	GMJoystickState joystickState;

	// keyboard
	GMbyte keyState[MAX_KEYS] = {0};
	GMbyte lastKeyState[MAX_KEYS] = {0};

	// mouse
	GMMouseState mouseState;
	GMWheelState wheelState;

	// implements
	IJoystickState* joystickImpl = nullptr;
	IMouseState* mouseImpl = nullptr;
	IKeyboardState* keyboardImpl = nullptr;
	IIMState* imImpl = nullptr;

	void updateKeymap(GMbyte*);
};

void GMInputPrivate::updateKeymap(GMbyte* map)
{
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(window->getContext());
	Display* display = context->getDisplay();
	XQueryKeymap(display, reinterpret_cast<char*>(map));
}

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
	d->updateKeymap(d->lastKeyState);
	d->wheelState.wheeled = false;
	d->wheelState.delta = 0;
	d->mouseState.downButton = d->mouseState.upButton = GMMouseButton_None;
	d->mouseState.moving = false;
}

IKeyboardState& GMInput::getKeyboardState()
{
	D(d);
	d->updateKeymap(d->keyState);
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

GMInput::Data& GMInput::dataRef()
{
	D(d);
	return *d;
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
			state.deltaX = rx - centerX;
			state.deltaY = ry - centerY;
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
	GMInput::Data* d = &m_host->dataRef();
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(d->window->getContext());
	Display* display = context->getDisplay();
	return isKeyDown(display, key, d->lastKeyState);
}

// 表示一个键是否按下一次，长按只算是一次
bool KeyboardStateImpl::keyTriggered(GMKey key)
{
	GMInput::Data* d = &m_host->dataRef();
	const GMXRenderContext* context = gm_cast<const GMXRenderContext*>(d->window->getContext());
	Display* display = context->getDisplay();
	return isKeyDown(display, key, d->keyState);
}

END_NS
