#ifndef __GM_LUA_IINPUT_META_H__
#define __GM_LUA_IINPUT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gminput.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IInputProxy)
	{
		GM_LUA_PROXY_FUNC(getKeyboardState);
		GM_LUA_PROXY_FUNC(getJoystickState);
		GM_LUA_PROXY_FUNC(getMouseState);
	};

	class IInputProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IInput, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IInputProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(IKeyboardStateProxy)
	{
		GM_LUA_PROXY_FUNC(keydown);
		GM_LUA_PROXY_FUNC(keyTriggered);
	};

	class IKeyboardStateProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IKeyboardState, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IKeyboardStateProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(IMouseStateProxy)
	{
		GM_LUA_PROXY_FUNC(state);
		GM_LUA_PROXY_FUNC(setDetectingMode);
	};

	class IMouseStateProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IMouseState, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IMouseStateProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(IJoystickStateProxy)
	{
		GM_LUA_PROXY_FUNC(vibrate);
		GM_LUA_PROXY_FUNC(state);
	};

	class IJoystickStateProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IJoystickState, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IJoystickStateProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif