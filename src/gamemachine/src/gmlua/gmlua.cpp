#include "stdafx.h"
#include "gmlua.h"
#include "gmluameta.h"
#include "foundation/utilities/tools.h"
#define L (d->luaState)

#define POP_GUARD() \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : m_L(__L) {}\
	~__PopGuard() { lua_pop(m_L, 1); }		\
	lua_State* m_L;							\
} __guard(this->getLuaCoreState());

#define POP_GUARD_(l) \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : m_L(__L) {}\
	~__PopGuard() { lua_pop(m_L, 1); }		\
	lua_State* m_L;							\
} __guard(l);

#define CHECK(lr) \
	if (lr.state != GMLuaStates::Ok)		\
	{										\
		lr.message = lua_tostring(L, -1);	\
		return lr;							\
	}

namespace
{
	template <typename T>
	void pushVector(const T& v, GMLuaCoreState* l)
	{
		GM_CHECK_LUA_STACK_BALANCE_(l, 1);
		lua_newtable(l);
		GMFloat4 f4;
		v.loadFloat4(f4);
		for (GMint i = 0; i < T::length(); i++)
		{
			lua_pushnumber(l, i);
			lua_pushnumber(l, f4[i]);
			GM_ASSERT(lua_istable(l, -3));
			lua_settable(l, -3);
		}
	}

	template <typename T>
	bool popVector(T& v, GMint index, GMLuaCoreState* l)
	{
		GM_CHECK_LUA_STACK_BALANCE_(l, 0);
		GMFloat4 f4(0, 0, 0, 0);
		GM_ASSERT(lua_istable(l, index));
		lua_pushnil(l);
		while (lua_next(l, index))
		{
			POP_GUARD_(l);
			if (!lua_isinteger(l, -2))
				return false;

			GMint key = lua_tointeger(l, -2);
			if (!lua_isnumber(l, -1))
				return false;

			f4[key - 1] = lua_tonumber(l, -1);
			// 防止越界
			if (key > T::length())
			{
				lua_next(l, index);
				break;
			}
		}
		v.setFloat4(f4);
		return true;
	}

	template <typename T>
	bool popVector(T& v, GMLuaCoreState* l)
	{
		GM_CHECK_LUA_STACK_BALANCE_(l, 0);
		GMint index = lua_gettop(l);
		return popVector(v, index, l);
	}

	bool popMatrix(GMMat4& v, GMint index, GMLuaCoreState* l)
	{
		GM_CHECK_LUA_STACK_BALANCE_(l, 0);
		GMVec4 v4[GMMat4::length()];
		GM_ASSERT(lua_istable(l, index));
		lua_pushnil(l);
		while (lua_next(l, index))
		{
			POP_GUARD_(l);
			if (!lua_isinteger(l, -2))
				return false;

			GMint key = lua_tointeger(l, -2);
			bool isVector = popVector(v4[key - 1], l);
			if (!isVector)
				return false;

			// 防止越界
			if (key > GMMat4::length())
			{
				lua_next(l, index);
				break;
			}
		}

		GMFloat16 f16;
		GMFloat4 f4[GMMat4::length()];
		for (decltype(GMMat4::length()) i = 0; i < GMMat4::length(); ++i)
		{
			v4[i].loadFloat4(f4[i]);
			f16[i] = f4[i];
		}
		v.setFloat16(f16);
		return true;
	}
}

void GMLuaFunctionRegister::setRegisterFunction(GMLua *l, const GMString& modname, GMLuaCFunction openf, bool isGlobal)
{
	GMLuaCoreState* cl = l->getLuaCoreState();
	GM_CHECK_LUA_STACK_BALANCE_(cl, 0);
	auto m = modname.toStdString();
	luaL_requiref(cl, m.data(), openf, isGlobal ? 1 : 0);
	lua_pop(cl, 1);
}

void GMLuaFunctionRegister::newLibrary(GMLuaCoreState *l, const GMLuaReg* functions)
{
	luaL_newlib(l, functions);
}

GMLua::GMLua()
{
	D(d);
	d->luaState = luaL_newstate();
	d->isWeakLuaStatePtr = false;
}

GMLua::GMLua(lua_State* l)
{
	D(d);
	d->luaState = l;
	d->isWeakLuaStatePtr = true;
}

GMLua::~GMLua()
{
	D(d);
	if (!d->isWeakLuaStatePtr && d->luaState)
		lua_close(d->luaState);
}

GMLuaResult GMLua::runFile(const char* file)
{
	D(d);
	GM_ASSERT(L);
	loadLibrary();
	GMLuaStates s = static_cast<GMLuaStates>(luaL_loadfile(L, file));
	GMLuaResult lr = { s };
	CHECK(lr);
	lr = { static_cast<GMLuaStates>(lua_pcall(L, 0, LUA_MULTRET, 0)) };
	CHECK(lr);
	return lr;
}

GMLuaResult GMLua::runBuffer(const GMBuffer& buffer)
{
	D(d);
	GM_ASSERT(L);
	loadLibrary();
	GMLuaStates s = static_cast<GMLuaStates>(luaL_loadbuffer(L, (const char*)buffer.buffer, buffer.size, 0));
	GMLuaResult lr = { s };
	CHECK(lr);
	lr = { static_cast<GMLuaStates>(lua_pcall(L, 0, LUA_MULTRET, 0)) };
	CHECK(lr);
	return lr;
}

GMLuaResult GMLua::runString(const GMString& string)
{
	D(d);
	GM_ASSERT(L);
	loadLibrary();
	std::string stdstr = string.toStdString();
	GMLuaStates s = static_cast<GMLuaStates>(luaL_dostring(L, stdstr.c_str()));
	GMLuaResult lr = { s };
	CHECK(lr);
	return lr;
}

void GMLua::setToGlobal(const char* name, const GMVariant& var)
{
	D(d);
	push(var);
	lua_setglobal(L, name);
}

bool GMLua::setToGlobal(const char* name, GMObject& obj)
{
	D(d);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;
	
	pushTable(obj);
	lua_setglobal(L, name);
	return true;
}

GMVariant GMLua::getFromGlobal(const char* name)
{
	D(d);
	POP_GUARD();
	lua_getglobal(L, name);
	return getTop();
}

bool GMLua::getFromGlobal(const char* name, GMObject& obj)
{
	D(d);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;

	POP_GUARD();
	lua_getglobal(L, name);
	if (!lua_istable(L, 1))
		return false;

	return popTable(obj);
}

GMLuaResult GMLua::protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args, GMVariant* returns, GMint nRet)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(nRet);
	GMLuaResult lr = pcall(functionName, args, nRet);
	CHECK(lr);
	for (GMint i = 0; i < nRet; i++)
	{
		if (returns[i].isObject())
			popTable(*returns[i].toObject());
		else
			returns[i] = getTop();
	}
	return lr;
}

void GMLua::loadLibrary()
{
	D(d);
	if (!d->libraryLoaded)
	{
		luaL_openlibs(L);
		luaapi::registerLib(this);
		d->libraryLoaded = true;
	}
}

GMLuaResult GMLua::pcall(const char* functionName, const std::initializer_list<GMVariant>& args, GMint nRet)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(nRet);
	lua_getglobal(L, functionName);
	for (const auto& var : args)
	{
		push(var);
	}

	GM_ASSERT(args.size() < std::numeric_limits<GMuint>::max());
	GMLuaResult lr = { (GMLuaStates)lua_pcall(L, (GMuint)args.size(), nRet, 0) };
	CHECK(lr);
	return lr;
}

void GMLua::pushTable(const GMObject& obj)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(1);
	lua_newtable(L);
	auto meta = obj.meta();
	GM_ASSERT(meta);
	for (const auto& member : *meta)
	{
		setTable(member.first.toStdString().c_str(), member.second);
	}
}

bool GMLua::popTable(GMObject& obj)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	return popTable(obj, lua_gettop(L));
}

bool GMLua::popTable(GMObject& obj, GMint index)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;

	if (!lua_istable(L, index))
	{
		gm_error("GMLua (getTable): lua object type is {0}. Table is expected.", { lua_typename(L, lua_type(L, index)) });
		return false;
	}

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
				case GMMetaMemberType::String:
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
						if (!popVector(v))
							return false;
					}
					break;
				case GMMetaMemberType::Vector3:
					{
						GMVec3& v = *static_cast<GMVec3*>(member.second.ptr);
						if (!popVector(v))
							return false;
					}
					break;
				case GMMetaMemberType::Vector4:
					{
						GMVec4& v = *static_cast<GMVec4*>(member.second.ptr);
						if (!popVector(v))
							return false;
					}
					break;
				case GMMetaMemberType::Matrix4x4:
					{
						GMMat4& mat = *static_cast<GMMat4*>(member.second.ptr);
						if (!popMatrix(mat))
							return false;
					}
					break;
				case GMMetaMemberType::Object:
					{
						GMObject* obj = static_cast<GMObject*>(member.second.ptr);
						if (!popTable(*obj))
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

bool GMLua::popVector(GMVec2& v)
{
	D(d);
	return ::popVector(v, d->luaState);
}

void GMLua::pushVector(const GMVec2& v)
{
	D(d);
	::pushVector(v, d->luaState);
}

bool GMLua::popVector(GMVec3& v)
{
	D(d);
	return ::popVector(v, d->luaState);
}

void GMLua::pushVector(const GMVec3& v)
{
	D(d);
	::pushVector(v, d->luaState);
}

bool GMLua::popVector(GMVec4& v)
{
	D(d);
	return ::popVector(v, d->luaState);
}

void GMLua::pushVector(const GMVec4& v)
{
	D(d);
	::pushVector(v, d->luaState);
}

void GMLua::pushMatrix(const GMMat4& v)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(1);
	lua_newtable(L);
	for (GMint i = 0; i < GMMat4::length(); i++)
	{
		lua_pushnumber(L, i);
		pushVector(v[i]);
		GM_ASSERT(lua_istable(L, -3));
		lua_settable(L, -3);
	}
}

bool GMLua::popMatrix(GMMat4& v)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	return ::popMatrix(v, lua_gettop(L), L);
}

void GMLua::push(const GMVariant& var)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(1);
	if (var.isInt())
	{
		lua_pushinteger(L, var.toInt());
	}
	else if (var.isFloat())
	{
		lua_pushnumber(L, var.toFloat());
	}
	else if (var.isBool())
	{
		lua_pushboolean(L, var.toBool());
	}
	else if (var.isString())
	{
		std::string str = var.toString().toStdString();
		lua_pushstring(L, str.c_str());
	}
	else if (var.isObject())
	{
		pushTable(*var.toObject());
	}
	else
	{
		gm_error(L"GMLua (push): variant type not supported");
		GM_ASSERT(false);
	}
}

void GMLua::setTable(const char* key, const GMObjectMember& value)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	lua_pushstring(L, key);

	switch (value.type)
	{
	case GMMetaMemberType::Int:
		lua_pushinteger(L, *static_cast<GMint*>(value.ptr));
		break;
	case GMMetaMemberType::Float:
		lua_pushnumber(L, *static_cast<GMfloat*>(value.ptr));
		break;
	case GMMetaMemberType::Boolean:
		lua_pushboolean(L, *static_cast<bool*>(value.ptr));
		break;
	case GMMetaMemberType::String:
		{
			std::string s = (static_cast<GMString*>(value.ptr))->toStdString();
			lua_pushstring(L, s.c_str());
		}
		break;
	case GMMetaMemberType::Vector2:
		{
			GMVec2& vec2 = *static_cast<GMVec2*>(value.ptr);
			pushVector(vec2);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Vector3:
		{
			GMVec3& vec3 = *static_cast<GMVec3*>(value.ptr);
			pushVector(vec3);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Vector4:
		{
			GMVec4& vec4 = *static_cast<GMVec4*>(value.ptr);
			pushVector(vec4);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Matrix4x4:
		{
			GMMat4& mat= *static_cast<GMMat4*>(value.ptr);
			pushMatrix(mat);
			GM_ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Object:
		pushTable(*static_cast<GMObject*>(value.ptr));
		break;
	default:
		GM_ASSERT(false);
		break;
	}

	GM_ASSERT(lua_istable(L, -3));
	lua_settable(L, -3);
}

GMVariant GMLua::getTop()
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	if (lua_isinteger(L, -1))
		return lua_tointeger(L, -1);
	if (lua_isnumber(L, -1))
		return lua_tonumber(L, -1);
	if (lua_isstring(L, -1))
		return GMString(lua_tostring(L, -1));
	if (lua_isboolean(L, -1))
		return lua_toboolean(L, -1) ? true : false;
	gm_error(L"GMLua (pop): type not supported");
	return GMVariant();
}