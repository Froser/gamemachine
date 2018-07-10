#ifndef __GM_LUA_GMDEBUGGER_META_H__
#define __GM_LUA_GMDEBUGGER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	class GMDebugger_Meta : public GMLuaFunctionRegister
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