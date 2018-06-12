#include "stdafx.h"
#include "gmlua.h"
#include "gmlua_functions.h"

#define L (d->luaState)

#define POP_GUARD() \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : m_L(__L) {}\
	~__PopGuard() { lua_pop(m_L, 1); }		\
	lua_State* m_L;							\
} __guard(*this);

GMLua::GMLua()
{
	D(d);
	L = luaL_newstate();
	d->ref = new GMuint(1);
}

GMLua::GMLua(lua_State* l)
{
	D(d);
	d->weakRef = true;
	L = l;
}

GMLua::~GMLua()
{
	D(d);
	if (!d->weakRef && d->ref)
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

GMLuaStatus GMLua::loadFile(const char* file)
{
	D(d);
	GM_ASSERT(L);
	loadLibrary();
	return (GMLuaStatus)(luaL_loadfile(L, file) || lua_pcall(L, 0, LUA_MULTRET, 0));
}

GMLuaStatus GMLua::loadBuffer(const GMBuffer& buffer)
{
	D(d);
	GM_ASSERT(L);
	loadLibrary();
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
	POP_GUARD();
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;

	lua_getglobal(L, name);
	if (!lua_istable(L, 1))
		return false;

	return getTable(obj);
}

GMLuaStatus GMLua::call(const char* functionName, const std::initializer_list<GMLuaVariable>& args)
{
	return callp(functionName, args, 0);
}

GMLuaStatus GMLua::call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable* returns, GMint nRet)
{
	GMLuaStatus result = callp(functionName, args, nRet);
	if (result == GMLuaStatus::OK)
	{
		for (GMint i = 0; i < nRet; i++)
		{
			returns[i] = pop();
		}
	}
	return result;
}

GMLuaStatus GMLua::call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMObject* returns, GMint nRet)
{
	GMLuaStatus result = callp(functionName, args, nRet);
	if (result == GMLuaStatus::OK)
	{
		for (GMint i = 0; i < nRet; i++)
		{
			POP_GUARD();
			if (!getTable(returns[i]))
				return GMLuaStatus::WRONG_TYPE;
		}
	}
	return result;
}

bool GMLua::invoke(const char* expr)
{
	D(d);
	return luaL_dostring(L, expr);
}

void GMLua::loadLibrary()
{
	D(d);
	luaL_openlibs(L);
	luaapi::register_functions(L);
}

void GMLua::callExceptionHandler(GMLuaStatus state, const char* msg)
{
	D(d);
	if (d->exceptionHandler)
		d->exceptionHandler->onException(state, msg);
	else
		gm_error("LUA error: %d, %s", (GMint)state, msg);
}

GMLuaStatus GMLua::callp(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMint nRet)
{
	D(d);
	lua_getglobal(L, functionName);
	for (const auto& var : args)
	{
		push(var);
	}

	GM_ASSERT(args.size() < std::numeric_limits<GMuint>::max());
	GMLuaStatus result = (GMLuaStatus)lua_pcall(L, (GMuint)args.size(), nRet, 0);
	if (result != GMLuaStatus::OK)
	{
		const char* msg = lua_tostring(L, -1);
		callExceptionHandler(result, msg);
		lua_pop(L, 1);
	}
	return result;
}

void GMLua::setTable(GMObject& obj)
{
	D(d);
	lua_newtable(L);
	auto meta = obj.meta();
	GM_ASSERT(meta);
	for (const auto& member : *meta)
	{
		push(member.first.toStdString().c_str(), member.second);
	}
}

bool GMLua::getTable(GMObject& obj)
{
	D(d);
	GMint index = lua_gettop(L);
	return getTable(obj, index);
}

bool GMLua::getTable(GMObject& obj, GMint index)
{
	D(d);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;

	GM_ASSERT(lua_istable(L, index));
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		POP_GUARD(); // Make sure pop every value
		GM_ASSERT(lua_isstring(L, -2));
		const char* key = lua_tostring(L, -2);
		for (const auto member : *meta)
		{
			if (member.first == key)
			{
				switch (member.second.type)
				{
				case GMMetaMemberType::GMString:
					*(static_cast<GMString*>(member.second.ptr)) = lua_tostring(L, -1);
					break;
				case GMMetaMemberType::Float:
					*(static_cast<GMfloat*>(member.second.ptr)) = lua_tonumber(L, -1);
					break;
				case GMMetaMemberType::Boolean:
					*(static_cast<bool*>(member.second.ptr)) = !!lua_toboolean(L, -1);
					break;
				case GMMetaMemberType::Int:
					*(static_cast<GMint*>(member.second.ptr)) = lua_tointeger(L, -1);
					break;
				case GMMetaMemberType::Vector2:
					{
						GMVec2& v = *static_cast<GMVec2*>(member.second.ptr);
						if (!getVector(v))
							return false;
					}
					break;
				case GMMetaMemberType::Vector3:
					{
						GMVec3& v = *static_cast<GMVec3*>(member.second.ptr);
						if (!getVector(v))
							return false;
					}
					break;
				case GMMetaMemberType::Vector4:
					{
						GMVec4& v = *static_cast<GMVec4*>(member.second.ptr);
						if (!getVector(v))
							return false;
					}
					break;
				case GMMetaMemberType::Matrix4x4:
					{
						GMMat4& mat = *static_cast<GMMat4*>(member.second.ptr);
						if (!getMatrix(mat))
							return false;
					}
					break;
				case GMMetaMemberType::Object:
					{
						GMObject* obj = static_cast<GMObject*>(member.second.ptr);
						if (!getTable(*obj))
							return false;
					}
					break;
				default:
					GM_ASSERT(false);
					break;
				}
			}
		}
	}

	return true;
}

void GMLua::push(const GMLuaVariable& var)
{
	D(d);
	switch (var.type)
	{
	case GMLuaVariableType::Int:
		lua_pushinteger(L, var.valInt);
		break;
	case GMLuaVariableType::Number:
		lua_pushnumber(L, var.valFloat);
		break;
	case GMLuaVariableType::Boolean:
		lua_pushboolean(L, var.valBoolean);
		break;
	case GMLuaVariableType::String:
		{
			std::string str = var.valPtrString->toStdString();
			lua_pushstring(L, str.c_str());
		}
		break;
	case GMLuaVariableType::Object:
		setTable(*var.valPtrObject);
		break;
	default:
		GM_ASSERT(false);
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
			GMVec2& vec2 = *static_cast<GMVec2*>(member.ptr);
			setVector(vec2);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Vector3:
		{
			GMVec3& vec3 = *static_cast<GMVec3*>(member.ptr);
			setVector(vec3);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Vector4:
		{
			GMVec4& vec4 = *static_cast<GMVec4*>(member.ptr);
			setVector(vec4);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Matrix4x4:
		{
			GMMat4& mat= *static_cast<GMMat4*>(member.ptr);
			setMatrix(mat);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Object:
		setTable(*static_cast<GMObject*>(member.ptr));
		break;
	default:
		GM_ASSERT(false);
		break;
	}

	GM_ASSERT(lua_istable(L, -3));
	lua_settable(L, -3);
}

GMLuaVariable GMLua::pop()
{
	D(d);
	POP_GUARD();
	if (lua_isinteger(L, -1))
		return lua_tointeger(L, -1);
	if (lua_isnumber(L, -1))
		return lua_tonumber(L, -1);
	if (lua_isstring(L, -1))
		return GMString(lua_tostring(L, -1));
	if (lua_isboolean(L, -1))
		return lua_toboolean(L, -1);
	GM_ASSERT(false);
	return 0;
}
