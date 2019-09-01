#include <gamemachine.h>
#include <gmlua.h>
using namespace gm;

class GameMachineLua : public GMLuaFunctionRegister
{
public:
	virtual void registerFunctions(GMLua* L) override;

private:
	static int regCallback(GMLuaCoreState *L);
};

GM_LUA_FUNC(add)
{
	GM_LUA_CHECK_ARG_COUNT(L, 2, "add");
	GMfloat b = luaapi::GMArgumentHelper::popArgument(L, "add").toFloat();
	GMfloat a = luaapi::GMArgumentHelper::popArgument(L, "add").toFloat();
	return GMReturnValues(L, a + b);
}

void GameMachineLua::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, "GameMachine", regCallback, true);
}

int GameMachineLua::regCallback(GMLuaCoreState *L)
{
	GMLuaReg func[] = {
		GM_LUA_DECLARATIONS(add),
		{ 0, 0 }
	};
	newLibrary(L, func);
	return 1;
}

int main(int argc, char* argv[])
{	
	GMLua lua;
	GameMachineLua().registerFunctions(&lua);
	GMLuaResult result = lua.runString("print(GameMachine.add(1, 2))");
	if (result.state == GMLuaStates::SyntaxError)
		gm_error("Syntax error: {0}", result.message);

	getchar();
	return 0;
}
