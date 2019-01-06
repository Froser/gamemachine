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
	// GM_META_FUNCTION(keydown);
	// GM_META_FUNCTION(keyTriggered);
	return true;
}

bool IMouseStateProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(mouseState);
	GM_META_FUNCTION(setDetectingMode);
	return true;
}

bool IJoystickStateProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	// GM_META_FUNCTION(joystickVibrate);
	// GM_META_FUNCTION(joystickState);
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
GM_LUA_PROXY_IMPL(IMouseStateProxy, mouseState)
{
	static const GMString s_invoker = "IMouseState.mouseState";
	GM_LUA_CHECK_ARG_COUNT(L, 1, "IMouseState.mouseState");
	IMouseStateProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, GMMouseStateProxy(self->mouseState()));
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