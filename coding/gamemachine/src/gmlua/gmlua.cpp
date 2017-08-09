#include "stdafx.h"
#include "gmlua.h"

#define POP_GUARD(d) \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : L(__L) {}	\
	~__PopGuard() { lua_pop(L, 1); }		\
	lua_State* L;							\
} __guard(d->luaState);

GMLua::GMLua()
{
	D(d);
	d->luaState = luaL_newstate();
	d->ref = new GMuint(1);
}

GMLua::~GMLua()
{
	D(d);
	if (d->ref)
	{
		(*d->ref)--;
		if (*d->ref == 0)
		{
			if (d->luaState)
				lua_close(d->luaState);
			delete d->ref;
			d->ref = nullptr;
			d->luaState = nullptr;
		}
	}
}

GMLua::GMLua(const GMLua& lua)
{
	*this = lua;
}

GMLua::GMLua(GMLua&& lua) noexcept
{
	*this = std::move(lua);
}

GMLua& GMLua::operator= (const GMLua& state)
{
	D(d);
	if (&state == this)
		return *this;
	D_OF(state_d, &state);
	d->luaState = state_d->luaState;
	d->ref = state_d->ref;
	(*d->ref)++;
	return *this;
}

GMLua& GMLua::operator= (GMLua&& state) noexcept
{
	D_OF(state_d, &state);
	swap(*this, state);
	return *this;
}

GMLuaStates GMLua::loadFile(const char* file)
{
	D(d);
	ASSERT(d->luaState);
	return (GMLuaStates)(luaL_loadfile(d->luaState, file) || lua_pcall(d->luaState, 0, LUA_MULTRET, 0));
}

GMLuaStates GMLua::loadBuffer(const GMBuffer& buffer)
{
	D(d);
	ASSERT(d->luaState);
	return (GMLuaStates)(luaL_loadbuffer(d->luaState, (const char*) buffer.buffer, buffer.size, 0) || lua_pcall(d->luaState, 0, LUA_MULTRET, 0));
}

void GMLua::setGlobal(const char* name, const GMLuaVariable& var)
{
	D(d);
	push(var);
	lua_setglobal(d->luaState, name);
}

void GMLua::call(const char* functionName, const std::initializer_list<GMLuaVariable>& args)
{
	D(d);
	lua_getglobal(d->luaState, functionName);
	GMint count = 0;
	for (const auto& var : args)
	{
		push(var);
		count++;
	}

	GMLuaStates result = (GMLuaStates)lua_pcall(d->luaState, count, 1, 0);
	if (result != GMLuaStates::OK)
	{
		const char* msg = lua_tostring(d->luaState, -1);
		callExceptionHandler(result, msg);
		lua_pop(d->luaState, 1);
	}
}

void GMLua::call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable* returns, GMint nRet)
{
	call(functionName, args);
	for (GMint i = 0; i < nRet; i++)
	{
		returns[i] = pop();
	}
}

void GMLua::callExceptionHandler(GMLuaStates state, const char* msg)
{
	D(d);
	if (d->exceptionHandler)
		d->exceptionHandler->onException(state, msg);
	else
		gm_error("LUA error: %d, %s", (GMint)state, msg);
}

void GMLua::push(const GMLuaVariable& var)
{
	D(d);
	switch (var.type)
	{
	case GMLuaVariableType::Number:
		lua_pushnumber(d->luaState, var.valDouble);
		break;
	case GMLuaVariableType::Boolean:
		lua_pushboolean(d->luaState, var.valBoolean);
		break;
	case GMLuaVariableType::String:
		{
			std::string str = var.valString.toStdString();
			lua_pushstring(d->luaState, str.c_str());
		}
		break;
	default:
		ASSERT(false);
		break;
	}
}

GMLuaVariable GMLua::pop()
{
	D(d);
	POP_GUARD(d);

	if (lua_isnumber(d->luaState, -1))
		return lua_tonumber(d->luaState, -1);
	if (lua_isstring(d->luaState, -1))
		return GMString(lua_tostring(d->luaState, -1));
	if (lua_isboolean(d->luaState, -1))
		return lua_toboolean(d->luaState, -1);
	ASSERT(false);
	return 0;
}