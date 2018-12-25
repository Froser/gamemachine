#ifndef __GM_LUA_GMGAMEWORLD_META_H__
#define __GM_LUA_GMGAMEWORLD_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmgameworld.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGameWorldProxy)
	{
		GM_LUA_PROXY;

		GMGameWorld* gameworld = nullptr;
		GMString __name = "GMGameWorld";
		GM_LUA_META_FUNCTION(__gc);
		GM_LUA_META_FUNCTION(renderScene);
		GM_LUA_META_FUNCTION(addObjectAndInit);
		GM_LUA_META_FUNCTION(addToRenderList);
	};

	class GMGameWorldProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMGameWorldProxy)
		GM_LUA_META_PROXY_FUNCTIONS(GMGameWorld, gameworld)

	public:
		GMGameWorldProxy(GMGameWorld* gameworld = nullptr);

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