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
} __guard(*this);

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

// 检查栈是否平衡
#define BEGIN_CHECK_STACK() auto __stack = lua_gettop(L);
#define END_CHECK_STACK(offset) GM_ASSERT(lua_gettop(L) == __stack + offset)

namespace
{
	template <typename T>
	void setVector(const T& v, GMLuaCoreState* l)
	{
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
	bool getVector(T& v, GMint index, GMLuaCoreState* l)
	{
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
	bool getVector(T& v, GMLuaCoreState* l)
	{
		GMint index = lua_gettop(l);
		return getVector(v, index, l);
	}

	template <typename T>
	bool getMatrix(T& v, GMLuaCoreState* l)
	{
		GMint index = lua_gettop(l);
		return getMatrix(v, index, l);
	}

	template <typename T>
	bool getMatrix(T& v, GMint index, GMLuaCoreState* l)
	{
		GMVec4 v4[T::length()];
		GM_ASSERT(lua_istable(l, index));
		lua_pushnil(l);
		while (lua_next(l, index))
		{
			POP_GUARD_(l);
			if (!lua_isinteger(l, -2))
				return false;

			GMint key = lua_tointeger(l, -2);
			bool isVector = getVector(v4[key - 1], l);
			if (!isVector)
				return false;

			// 防止越界
			if (key > T::length())
			{
				lua_next(l, index);
				break;
			}
		}

		GMFloat16 f16;
		GMFloat4 f4[T::length()];
		for (decltype(T::length()) i = 0; i < T::length(); ++i)
		{
			v4[i].loadFloat4(f4[i]);
			f16[i] = f4[i];
		}
		v.setFloat16(f16);
		return true;
	}
}

void GMLuaFunctionRegister::setRegisterFunction(GMLuaCoreState *l, const GMString& modname, GMLuaCFunction openf, bool isGlobal)
{
	auto m = modname.toStdString();
	luaL_requiref(l, m.data(), openf, isGlobal ? 1 : 0);
	lua_pop(l, 1);
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

void GMLua::setGlobal(const char* name, const GMVariant& var)
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

GMVariant GMLua::getGlobal(const char* name)
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

GMLuaResult GMLua::protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args)
{
	D(d);
	return pcall(functionName, args, 0);
}

GMLuaResult GMLua::protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args, GMVariant* returns, GMint nRet)
{
	D(d);
	BEGIN_CHECK_STACK();
	GMLuaResult lr = pcall(functionName, args, nRet);
	CHECK(lr);
	for (GMint i = 0; i < nRet; i++)
	{
		returns[i] = pop();
	}
	END_CHECK_STACK(0);
	return lr;
}

GMLuaResult GMLua::protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args, GMObject* returns, GMint nRet)
{
	D(d);
	BEGIN_CHECK_STACK();
	GMLuaResult lr = pcall(functionName, args, nRet);
	CHECK(lr);
	for (GMint i = 0; i < nRet; i++)
	{
		POP_GUARD();
		if (!getTable(returns[i]))
		{
			lr.state = GMLuaStates::WrongType;
			const char* errmsg = "cannot convert to GMObject";
			lr.message = errmsg;
			gm_error(L"GMLua (protectedCall): " + GMString(errmsg));
		}
	}
	END_CHECK_STACK(0);
	return lr;
}

void GMLua::loadLibrary()
{
	D(d);
	if (!d->libraryLoaded)
	{
		luaL_openlibs(L);
		registerLibraries();
		d->libraryLoaded = true;
	}
}

void GMLua::registerLibraries()
{
	D(d);
	luaapi::registerLib(L);
}

GMLuaResult GMLua::pcall(const char* functionName, const std::initializer_list<GMVariant>& args, GMint nRet)
{
	D(d);
	BEGIN_CHECK_STACK();
	lua_getglobal(L, functionName);
	for (const auto& var : args)
	{
		push(var);
	}

	GM_ASSERT(args.size() < std::numeric_limits<GMuint>::max());
	GMLuaResult lr = { (GMLuaStates)lua_pcall(L, (GMuint)args.size(), nRet, 0) };
	CHECK(lr);
	END_CHECK_STACK(nRet);
	return lr;
}

void GMLua::setTable(const GMObject& obj)
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
	return getTable(obj, lua_gettop(L));
}

bool GMLua::getTable(GMObject& obj, GMint index)
{
	D(d);
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

bool GMLua::getVector(GMVec2& v)
{
	return ::getVector(v, *this);
}

void GMLua::setVector(const GMVec2& v)
{
	::setVector(v, *this);
}

bool GMLua::getVector(GMVec3& v)
{
	return ::getVector(v, *this);
}

void GMLua::setVector(const GMVec3& v)
{
	::setVector(v, *this);
}

bool GMLua::getVector(GMVec4& v)
{
	return ::getVector(v, *this);
}

void GMLua::setVector(const GMVec4& v)
{
	::setVector(v, *this);
}

void GMLua::setMatrix(const GMMat4& v)
{
	D(d);
	lua_newtable(L);
	for (GMint i = 0; i < GMMat4::length(); i++)
	{
		lua_pushnumber(L, i);
		setVector(v[i]);
		GM_ASSERT(lua_istable(L, -3));
		lua_settable(L, -3);
	}
}

bool GMLua::getMatrix(GMMat4& v)
{
	return ::getMatrix(v, *this);
}

void GMLua::push(const GMVariant& var)
{
	D(d);
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
	else if (var.isPointer())
	{
		setTable(* (GMObject*)var.toPointer());
	}
	else
	{
		gm_error(L"GMLua (push): variant type not supported");
		GM_ASSERT(false);
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
	case GMMetaMemberType::String:
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

GMVariant GMLua::pop()
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
		return lua_toboolean(L, -1) ? true : false;
	gm_error(L"GMLua (pop): variant type not supported");
	return GMVariant();
}