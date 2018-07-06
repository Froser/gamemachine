#ifndef __GM_LUA_GAMEMACHINE_META_H__
#define __GM_LUA_GAMEMACHINE_META_H__
#include <gmcommon.h>
#include <gmlua.h>

#define GM_LUA_DECLARATIONS(name) { #name, name }

BEGIN_NS

namespace luaapi
{
	class GameMachine_Meta : public ILuaFunctionRegister
	{
	public:
		virtual void registerFunctions(lua_State* L) override;

	private:
		static int registerFunction(lua_State *L);
		static const char* Name;
	};
}

END_NS
#endif