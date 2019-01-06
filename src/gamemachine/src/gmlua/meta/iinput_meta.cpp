#include "stdafx.h"
#include "iinput_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include <gminput.h>

using namespace gm::luaapi;
class GMMouseStateProxy : public GMObject
{
	GM_DECLARE_PRIVATE_FROM_STRUCT(GMMouseStateProxy, GMMouseState)

public:
	GMMouseStateProxy(const GMMouseState& state)
	{
		D(d);
		*d = state;
	}

protected:
	virtual bool registerMeta() override
	{
		GM_META(deltaX);
		GM_META(deltaY);
		GM_META(posX);
		GM_META(posY);
		GM_META_WITH_TYPE(downButton, GMMetaMemberType::Int);
		GM_META_WITH_TYPE(upButton, GMMetaMemberType::Int);
		GM_META_WITH_TYPE(triggerButton, GMMetaMemberType::Int);
		GM_META(wheeled);
		GM_META(wheeledDelta);
		GM_META(moving);
		return true;
	}
};

struct GMJoystickState_
{
	bool valid;
	GMint32 buttons;
	GMint32 leftTrigger;
	GMint32 rightTrigger;
	GMint32 thumbLX;
	GMint32 thumbLY;
	GMint32 thumbRX;
	GMint32 thumbRY;
};

class GMJoystickStateProxy : public GMObject
{
	GM_DECLARE_PRIVATE_FROM_STRUCT(GMJoystickStateProxy, GMJoystickState_)

public:
	GMJoystickStateProxy(const GMJoystickState& state)
	{
		D(d);
		d->valid = state.valid;
		d->buttons = state.buttons;
		d->leftTrigger = static_cast<GMint32>(state.leftTrigger);
		d->rightTrigger = static_cast<GMint32>(state.rightTrigger);
		d->thumbLX = static_cast<GMint32>(state.thumbLX);
		d->thumbLY = static_cast<GMint32>(state.thumbLY);
		d->thumbRX = static_cast<GMint32>(state.thumbRX);
		d->thumbRY = static_cast<GMint32>(state.thumbRY);
	}

protected:
	virtual bool registerMeta() override
	{
		GM_META(valid);
		GM_META(buttons);
		GM_META(leftTrigger);
		GM_META(rightTrigger);
		GM_META(thumbLX);
		GM_META(thumbLY);
		GM_META(thumbRX);
		GM_META(thumbRY);
		return true;
	}
};

//////////////////////////////////////////////////////////////////////////
bool IInputProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(getKeyboardState);
	GM_META_FUNCTION(getJoystickState);
	GM_META_FUNCTION(getMouseState);
	return true;
}

bool IKeyboardStateProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(keydown);
	GM_META_FUNCTION(keyTriggered);
	return true;
}

bool IMouseStateProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(state);
	GM_META_FUNCTION(setDetectingMode);
	return true;
}

bool IJoystickStateProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(vibrate);
	GM_META_FUNCTION(state);
	return true;
}

//////////////////////////////////////////////////////////////////////////
/*
 * getKeyboardState([self])
 */
GM_LUA_PROXY_IMPL(IInputProxy, getKeyboardState)
{
	static const GMString s_invoker = "IInput.getKeyboardState";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "IInput.getKeyboardState");
	IInputProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, IKeyboardStateProxy(&self->getKeyboardState()));
	return GMReturnValues();
}

/*
 * getJoystickState([self])
 */
GM_LUA_PROXY_IMPL(IInputProxy, getJoystickState)
{
	static const GMString s_invoker = "IInput.getJoystickState";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "IInput.getJoystickState");
	IInputProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, IJoystickStateProxy(&self->getJoystickState()));
	return GMReturnValues();
}

/*
 * getMouseState([self])
 */
GM_LUA_PROXY_IMPL(IInputProxy, getMouseState)
{
	static const GMString s_invoker = "IInput.getMouseState";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "IInput.getMouseState");
	IInputProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, IMouseStateProxy(&self->getMouseState()));
	return GMReturnValues();
}

/*
 * mouseState([self])
 */
GM_LUA_PROXY_IMPL(IMouseStateProxy, state)
{
	static const GMString s_invoker = "IMouseState.state";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "IMouseState.state");
	IMouseStateProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, GMMouseStateProxy(self->state()));
	return GMReturnValues();
}

/*
 * mouseState([self], bool)
 */
GM_LUA_PROXY_IMPL(IMouseStateProxy, setDetectingMode)
{
	static const GMString s_invoker = "IMouseState.setDetectingMode";
	GM_LUA_CHECK_ARG_COUNT(L, 2, "IMouseState.setDetectingMode");
	IMouseStateProxy self;
	bool b = GMArgumentHelper::popArgument(L, s_invoker).toBool(); //bool
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setDetectingMode(b);
	return GMReturnValues();
}

/*
 * keydown([self], key)
 */
GM_LUA_PROXY_IMPL(IKeyboardStateProxy, keydown)
{
	static const GMString s_invoker = "IKeyboardState.keydown";
	GM_LUA_CHECK_ARG_COUNT(L, 2, "IKeyboardState.keydown");
	IKeyboardStateProxy self;
	GMKey key = static_cast<GMKey>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //key
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		GMReturnValues(L, self->keydown(key));
	return GMReturnValues();
}

/*
 * keyTriggered([self], key)
 */
GM_LUA_PROXY_IMPL(IKeyboardStateProxy, keyTriggered)
{
	static const GMString s_invoker = "IKeyboardState.keydown";
	GM_LUA_CHECK_ARG_COUNT(L, 2, "IKeyboardState.keydown");
	IKeyboardStateProxy self;
	GMKey key = static_cast<GMKey>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //key
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, self->keyTriggered(key));
	return GMReturnValues();
}

/*
 * joystickVibrate([self], leftMotorSpeed, rightMotorSpeed)
 */
GM_LUA_PROXY_IMPL(IJoystickStateProxy, vibrate)
{
	static const GMString s_invoker = "IJoystickState.vibrate";
	GM_LUA_CHECK_ARG_COUNT(L, 3, "IJoystickState.vibrate");
	IJoystickStateProxy self;
	GMushort right = static_cast<GMushort>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //rightMotorSpeed
	GMushort left = static_cast<GMushort>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //leftMotorSpeed
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->vibrate(left, right);
	return GMReturnValues();
}

/*
 * state([self])
 */
GM_LUA_PROXY_IMPL(IJoystickStateProxy, state)
{
	static const GMString s_invoker = "IJoystickState.state";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "IJoystickState.state");
	IJoystickStateProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, GMJoystickStateProxy(self->state()));
	return GMReturnValues();
}