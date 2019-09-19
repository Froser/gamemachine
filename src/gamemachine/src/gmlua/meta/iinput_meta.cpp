#include "stdafx.h"
#include "iinput_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include <gminput.h>
#include "gmobject_meta.h"

BEGIN_NS

namespace luaapi
{
	GM_LUA_PRIVATE_CLASS_FROM(GMMouseStateProxy, GMMouseState)
	class GMMouseStateProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMMouseStateProxy)

	public:
		GMMouseStateProxy(const GMMouseState& state)
		{
			GM_CREATE_DATA();
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

	GM_PRIVATE_OBJECT_UNALIGNED(GMJoystickStateProxy)
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
		GM_DECLARE_PRIVATE(GMJoystickStateProxy)

	public:
		GMJoystickStateProxy(const GMJoystickState& state)
		{
			GM_CREATE_DATA();
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
	GM_PRIVATE_OBJECT_UNALIGNED(IInputProxy)
	{
		GM_LUA_PROXY_FUNC(getKeyboardState);
		GM_LUA_PROXY_FUNC(getJoystickState);
		GM_LUA_PROXY_FUNC(getMouseState);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(IKeyboardStateProxy)
	{
		GM_LUA_PROXY_FUNC(keydown);
		GM_LUA_PROXY_FUNC(keyTriggered);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(IMouseStateProxy)
	{
		GM_LUA_PROXY_FUNC(state);
		GM_LUA_PROXY_FUNC(setDetectingMode);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(IJoystickStateProxy)
	{
		GM_LUA_PROXY_FUNC(vibrate);
		GM_LUA_PROXY_FUNC(state);
	};

	bool IInputProxy::registerMeta()
	{
		GM_META_FUNCTION(getKeyboardState);
		GM_META_FUNCTION(getJoystickState);
		GM_META_FUNCTION(getMouseState);
		return Base::registerMeta();
	}

	IInputProxy::IInputProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	IInputProxy::~IInputProxy()
	{

	}

	bool IKeyboardStateProxy::registerMeta()
	{
		GM_META_FUNCTION(keydown);
		GM_META_FUNCTION(keyTriggered);
		return Base::registerMeta();
	}

	IKeyboardStateProxy::IKeyboardStateProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	bool IMouseStateProxy::registerMeta()
	{
		GM_META_FUNCTION(state);
		GM_META_FUNCTION(setDetectingMode);
		return Base::registerMeta();
	}

	IMouseStateProxy::IMouseStateProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	bool IJoystickStateProxy::registerMeta()
	{
		GM_META_FUNCTION(vibrate);
		GM_META_FUNCTION(state);
		return Base::registerMeta();
	}

	IJoystickStateProxy::IJoystickStateProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

#define NAME "IInput"
	//////////////////////////////////////////////////////////////////////////
	/*
	 * getKeyboardState([self])
	 */
	GM_LUA_PROXY_IMPL(IInputProxy, getKeyboardState)
	{
		GMLuaArguments args(L, NAME ".getKeyboardState", { GMMetaMemberType::Object } );
		IInputProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			IKeyboardStateProxy proxy(L);
			proxy.set(&self->getKeyboardState());
			return gm::GMReturnValues(L, proxy);
		}
		return gm::GMReturnValues();
	}

	/*
	 * getJoystickState([self])
	 */
	GM_LUA_PROXY_IMPL(IInputProxy, getJoystickState)
	{
		GMLuaArguments args(L, NAME ".getJoystickState", { GMMetaMemberType::Object });
		IInputProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			IJoystickStateProxy proxy(L);
			proxy.set(&self->getJoystickState());
			return gm::GMReturnValues(L, proxy);
		}
		return gm::GMReturnValues();
	}

	/*
	 * getMouseState([self])
	 */
	GM_LUA_PROXY_IMPL(IInputProxy, getMouseState)
	{
		GMLuaArguments args(L, NAME ".getMouseState", { GMMetaMemberType::Object });
		IInputProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			IMouseStateProxy proxy(L);
			proxy.set(&self->getMouseState());
			return gm::GMReturnValues(L, proxy);
		}
		return gm::GMReturnValues();
	}

#undef NAME
#define NAME "IMouseState"
	/*
	 * mouseState([self])
	 */
	GM_LUA_PROXY_IMPL(IMouseStateProxy, state)
	{
		GMLuaArguments args(L, NAME ".state", { GMMetaMemberType::Object });
		IMouseStateProxy self(L);
		args.getArgument(0, &self);
		if (self)
			return gm::GMReturnValues(L, GMMouseStateProxy(self->state()));
		return gm::GMReturnValues();
	}

	/*
	 * mouseState([self], bool)
	 */
	GM_LUA_PROXY_IMPL(IMouseStateProxy, setDetectingMode)
	{
		GMLuaArguments args(L, NAME ".setDetectingMode", { GMMetaMemberType::Object, GMMetaMemberType::Boolean });
		IMouseStateProxy self(L);
		args.getArgument(0, &self);
		bool b = args.getArgument(1).toBool();
		if (self)
			self->setDetectingMode(b);
		return gm::GMReturnValues();
	}

#undef NAME
#define NAME "IKeyboardState"
	/*
	 * keydown([self], key)
	 */
	GM_LUA_PROXY_IMPL(IKeyboardStateProxy, keydown)
	{
		GMLuaArguments args(L, NAME ".keydown", { GMMetaMemberType::Object, GMMetaMemberType::Int });
		IKeyboardStateProxy self(L);
		args.getArgument(0, &self);
		GMKey key = static_cast<GMKey>(args.getArgument(1).toInt());
		if (self)
			gm::GMReturnValues(L, self->keydown(key));
		return gm::GMReturnValues();
	}

	/*
	 * keyTriggered([self], key)
	 */
	GM_LUA_PROXY_IMPL(IKeyboardStateProxy, keyTriggered)
	{
		GMLuaArguments args(L, NAME ".keydown", { GMMetaMemberType::Object, GMMetaMemberType::Int });
		IKeyboardStateProxy self(L);
		args.getArgument(0, &self);
		GMKey key = static_cast<GMKey>(args.getArgument(1).toInt());
		if (self)
			return gm::GMReturnValues(L, self->keyTriggered(key));
		return gm::GMReturnValues();
	}

#undef NAME
#define NAME "IJoystickState"
	/*
	 * joystickVibrate([self], leftMotorSpeed, rightMotorSpeed)
	 */
	GM_LUA_PROXY_IMPL(IJoystickStateProxy, vibrate)
	{
		GMLuaArguments args(L, NAME ".vibrate", { GMMetaMemberType::Object, GMMetaMemberType::Int, GMMetaMemberType::Int });
		IJoystickStateProxy self(L);
		args.getArgument(0, &self);
		GMushort left = static_cast<GMushort>(args.getArgument(1).toInt());
		GMushort right = static_cast<GMushort>(args.getArgument(2).toInt());
		if (self)
			self->vibrate(left, right);
		return gm::GMReturnValues();
	}

	/*
	 * state([self])
	 */
	GM_LUA_PROXY_IMPL(IJoystickStateProxy, state)
	{
		GMLuaArguments args(L, NAME "IJoystickState.state", { GMMetaMemberType::Object });
		IJoystickStateProxy self(L);
		args.getArgument(0, &self);
		if (self)
			return gm::GMReturnValues(L, GMJoystickStateProxy(self->state()));
		return gm::GMReturnValues();
	}
}
END_NS