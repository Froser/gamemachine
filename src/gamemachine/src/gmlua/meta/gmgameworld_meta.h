﻿#ifndef __GM_LUA_GMGAMEWORLD_META_H__
#define __GM_LUA_GMGAMEWORLD_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmgameworld.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGameWorldProxy)
	{
		GM_LUA_PROXY(GMGameWorld);
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(renderScene);
		GM_LUA_PROXY_FUNC(addObjectAndInit);
		GM_LUA_PROXY_FUNC(addToRenderList);
	};

	class GMGameWorldProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMGameWorldProxy, GMGameWorld)

	protected:
		virtual bool registerMeta() override;
	};

	class GMGameWorld_Meta : public GMLuaFunctionRegister
	{
	public:
		virtual void registerFunctions(GMLua* L) override;

	private:
		static int regCallback(GMLuaCoreState *L);
		static const char* Name;
	};
}

END_NS
#endif