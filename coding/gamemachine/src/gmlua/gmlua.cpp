#include "stdafx.h"
#include "gmlua.h"
#include "foundation/linearmath.h"

#define L (d->luaState)

#define POP_GUARD(d) \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : m_L(__L) {}\
	~__PopGuard() { lua_pop(m_L, 1); }		\
	lua_State* m_L;							\
} __guard(L);

GM_PRIVATE_OBJECT(GMLua_Vector2)
{
	GMfloat x;
	GMfloat y;
};

class GMLua_Vector2 : public GMObject
{
	DECLARE_PRIVATE(GMLua_Vector2)

	GM_BEGIN_META_MAP
		GM_META(x, GMMetaMemberType::Float)
		GM_META(y, GMMetaMemberType::Float)
	GM_END_META_MAP

public:
	GMLua_Vector2(GMfloat x, GMfloat y)
	{
		D(d);
		d->x = x;
		d->y = y;
	}
};

GMLua::GMLua()
{
	D(d);
	L = luaL_newstate();
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
			if (L)
				lua_close(L);
			delete d->ref;
			d->ref = nullptr;
			L = nullptr;
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

GMLuaStatus GMLua::loadFile(const char* file)
{
	D(d);
	ASSERT(L);
	return (GMLuaStatus)(luaL_loadfile(L, file) || lua_pcall(L, 0, LUA_MULTRET, 0));
}

GMLuaStatus GMLua::loadBuffer(const GMBuffer& buffer)
{
	D(d);
	ASSERT(L);
	return (GMLuaStatus)(luaL_loadbuffer(L, (const char*) buffer.buffer, buffer.size, 0) || lua_pcall(L, 0, LUA_MULTRET, 0));
}

void GMLua::setGlobal(const char* name, const GMLuaVariable& var)
{
	D(d);
	push(var);
	lua_setglobal(L, name);
}

bool GMLua::setGlobal(const char* name, GMObject& obj)
{
	D(d);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;
	
	setTable(obj);
	lua_setglobal(L, name);
	return true;
}

GMLuaVariable GMLua::getGlobal(const char* name)
{
	D(d);
	lua_getglobal(L, name);
	return pop();
}

bool GMLua::getGlobal(const char* name, GMObject& obj)
{
	D(d);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;

	lua_getglobal(L, name);
	GMint type = lua_type(L, 1);
	if (type != LUA_TTABLE)
		return false;

	GMint index = lua_gettop(L);
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		lua_pushvalue(L, -2);
		ASSERT(lua_type(L, -1) == LUA_TSTRING);
		const char* key = lua_tostring(L, -1);
		for (const auto member : *meta)
		{
			if (strEqual(member.first, key))
			{
				switch ( member.second.type )
				{
				case GMMetaMemberType::GMString:
					*(static_cast<GMString*>(member.second.ptr)) = lua_tostring(L, -2);
					break;
				case GMMetaMemberType::Float:
					*(static_cast<GMfloat*>(member.second.ptr)) = lua_tonumber(L, -2);
					break;
				case GMMetaMemberType::Boolean:
					*(static_cast<bool*>(member.second.ptr)) = !!lua_toboolean(L, -2);
					break;
				case GMMetaMemberType::Int:
					*(static_cast<GMint*>(member.second.ptr)) = lua_tointeger(L, -2);
					break;
				case GMMetaMemberType::Vector2:
				{
					ASSERT(false);
					break;
				}
				default:
					ASSERT(false);
					break;
				}
			}
		}

		lua_pop(L, 2);
	}
	return true;
}

void GMLua::call(const char* functionName, const std::initializer_list<GMLuaVariable>& args)
{
	D(d);
	lua_getglobal(L, functionName);
	GMint count = 0;
	for (const auto& var : args)
	{
		push(var);
		count++;
	}

	GMLuaStatus result = (GMLuaStatus)lua_pcall(L, count, 1, 0);
	if (result != GMLuaStatus::OK)
	{
		const char* msg = lua_tostring(L, -1);
		callExceptionHandler(result, msg);
		lua_pop(L, 1);
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

void GMLua::callExceptionHandler(GMLuaStatus state, const char* msg)
{
	D(d);
	if (d->exceptionHandler)
		d->exceptionHandler->onException(state, msg);
	else
		gm_error("LUA error: %d, %s", (GMint)state, msg);
}

void GMLua::setTable(GMObject& obj)
{
	D(d);
	lua_newtable(L);
	auto meta = obj.meta();
	ASSERT(meta);
	for (const auto& member : *meta)
	{
		push(member.first, member.second);
	}
}

void GMLua::push(const GMLuaVariable& var)
{
	D(d);
	switch (var.type)
	{
	case GMLuaVariableType::Number:
		lua_pushnumber(L, var.valDouble);
		break;
	case GMLuaVariableType::Boolean:
		lua_pushboolean(L, var.valBoolean);
		break;
	case GMLuaVariableType::String:
		{
			std::string str = var.valString.toStdString();
			lua_pushstring(L, str.c_str());
		}
		break;
	default:
		ASSERT(false);
		break;
	}
}

void GMLua::push(const char* name, const GMObjectMember& member)
{
	D(d);
	lua_pushstring(L, name);

	switch (member.type)
	{
	case GMMetaMemberType::Int:
		lua_pushinteger(L, *static_cast<GMint*>(member.ptr));
		break;
	case GMMetaMemberType::Float:
		lua_pushnumber(L, *static_cast<GMfloat*>(member.ptr));
		break;
	case GMMetaMemberType::Boolean:
		lua_pushboolean(L, *static_cast<bool*>(member.ptr));
		break;
	case GMMetaMemberType::GMString:
		{
			std::string value = (static_cast<GMString*>(member.ptr))->toStdString();
			lua_pushstring(L, value.c_str());
		}
		break;
	case GMMetaMemberType::Vector2:
		{
			linear_math::Vector2& vec2 = *static_cast<linear_math::Vector2*>(member.ptr);
			GMLua_Vector2 lua_vec2(vec2[0], vec2[1]);
			setTable(lua_vec2);
			ASSERT(lua_type(L, -1) == LUA_TTABLE);
		}
		break;
	default:
		ASSERT(false);
		break;
	}

	lua_settable(L, -3);
}

GMLuaVariable GMLua::pop()
{
	D(d);
	POP_GUARD(d);

	if (lua_isnumber(L, -1))
		return lua_tonumber(L, -1);
	if (lua_isstring(L, -1))
		return GMString(lua_tostring(L, -1));
	if (lua_isboolean(L, -1))
		return lua_toboolean(L, -1);
	ASSERT(false);
	return 0;
}