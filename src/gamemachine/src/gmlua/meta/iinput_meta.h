#ifndef __GM_LUA_IINPUT_META_H__
#define __GM_LUA_IINPUT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gminput.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(IInputProxy);
	class IInputProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IInput, GMAnyProxy)
		GM_DECLARE_PRIVATE(IInputProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IInputProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		~IInputProxy();

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_CLASS(IKeyboardStateProxy);
	class IKeyboardStateProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IKeyboardState, GMAnyProxy)
		GM_DECLARE_PRIVATE(IKeyboardStateProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IKeyboardStateProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_CLASS(IMouseStateProxy);
	class IMouseStateProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IMouseState, GMAnyProxy)
		GM_DECLARE_PRIVATE(IMouseStateProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IMouseStateProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_CLASS(IJoystickStateProxy);
	class IJoystickStateProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IJoystickState, GMAnyProxy)
		GM_DECLARE_PRIVATE(IJoystickStateProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IJoystickStateProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif