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
		lua_pop(L, 1);						\
		return lr;							\
	}

namespace
{
	class Runtimes
	{
	public:
		~Runtimes()
		{
			for (auto runtime : runtimes)
			{
				GM_delete(runtime.second);
			}
			GMClearSTLContainer(runtimes);
		}

		GMLuaRuntime* operator[](GMLuaCoreState* s)
		{
			GMLuaRuntime* runtime = runtimes[s];
			if (!runtime)
			{
				runtime = new GMLuaRuntime();
				runtimes[s] = runtime;
			}
			return runtime;
		}

	private:
		Map<GMLuaCoreState*, GMLuaRuntime*> runtimes;
	};

	template <typename T>
	void pushVector(const T& v, GMLuaCoreState* l)
	{
		GM_CHECK_LUA_STACK_BALANCE_(l, 1);
		lua_newtable(l);
		GMFloat4 f4;
		v.loadFloat4(f4);
		for (GMint32 i = 0; i < T::length(); i++)
		{
			lua_pushnumber(l, i);
			lua_pushnumber(l, f4[i]);
			GM_ASSERT(lua_istable(l, -3));
			lua_settable(l, -3);
		}
	}

	template <typename T>
	bool popVector(T& v, GMint32 index, GMLuaCoreState* l)
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

			GMint32 key = lua_tointeger(l, -2);
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
		GMint32 index = lua_gettop(l);
		return popVector(v, index, l);
	}

	bool popMatrix(GMMat4& v, GMint32 index, GMLuaCoreState* l)
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

			GMint32 key = lua_tointeger(l, -2);
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

	void getMetaTableAndName(const GMObject& obj, GMString& metatableName, const GMObject** metatable)
	{
		// 看看成员是否有GM_LUA_PROXY_METATABLE_NAME，如果有，设置它为元表，否则，设置自己为元表
		static const GMString s_metatableNameKw = L"__name";

		auto meta = obj.meta();
		for (const auto& member : *meta)
		{
			if (member.first == s_metatableNameKw)
				metatableName = *static_cast<GMString*>(member.second.ptr);
		}
		if (metatable)
			*metatable = &obj;
	}

	template <typename T, GMsize_t sz>
	bool contains(const T (&arr)[sz], const T& t)
	{
		for (GMsize_t i = 0; i < sz; ++i)
		{
			if (arr[i] == t)
				return true;
		}
		return false;
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
	// 内部通过sizeof求得functions数组大小，有个size_t->int转换，编译器会抛出警告
	// 实际上，截断几乎不可能发生的，因此消除这个警告
#pragma warning(push)
#pragma warning(disable:4309)
	luaL_newlib(l, functions); 
#pragma warning(pop)
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
	GMLuaStates s = static_cast<GMLuaStates>(luaL_loadbuffer(L, (const char*)buffer.getData(), buffer.getSize(), 0));
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
	
	pushNewTable(obj);
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

GMLuaResult GMLua::protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args, GMVariant* returns, GMint32 nRet)
{
	D(d);
	GMLuaResult lr = pcall(functionName, args, nRet);

	GM_CHECK_LUA_STACK_BALANCE(-nRet);
	if (returns)
	{
		for (GMint32 i = 0; i < nRet; i++)
		{
			if (returns[i].isObject())
			{
				POP_GUARD();
				popTable(*returns[i].toObject());
			}
			else
			{
				POP_GUARD();
				returns[i] = getTop();
			}
		}
	}

	if (lr.state != GMLuaStates::Ok)
	{
		gm_warning(gm_dbg_wrap("{0}"), lr.message);
	}

	return lr;
}

GMLuaResult GMLua::protectedCall(GMLuaReference funcRef, const std::initializer_list<GMVariant>& args, GMVariant* returns, GMint32 nRet)
{
	D(d);
	lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
	return protectedCall(nullptr, args, returns, nRet);
}

void GMLua::freeReference(GMLuaReference ref)
{
	D(d);
	luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

GMLuaRuntime* GMLua::getRuntime(GMLuaCoreState* s)
{
	static Runtimes runtimes;
	return runtimes[s];
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

GMLuaResult GMLua::pcall(const char* functionName, const std::initializer_list<GMVariant>& args, GMint32 nRet)
{
	D(d);
	if (functionName)
		lua_getglobal(L, functionName);

	for (const auto& var : args)
	{
		push(var);
	}

	GMLuaResult lr = { (GMLuaStates)lua_pcall(L, gm_sizet_to_uint(args.size()), nRet, 0) };
	CHECK(lr);
	return lr;
}

void GMLua::setEachMetaMember(const GMObject& obj)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	auto meta = obj.meta();
	if (meta)
	{
		for (const auto& member : *meta)
		{
			// 不将'__'开头的函数放入普通表
			if (!member.first.startsWith(L"__") || member.second.type != GMMetaMemberType::Function)
				setTable(member.first.toStdString().c_str(), member.second);
		}
	}
}

void GMLua::pushNewTable(const GMObject& obj, bool setmt)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(1);
	lua_newtable(L);
	setEachMetaMember(obj);
	if (setmt)
		setMetatable(obj);
}

void GMLua::setMetatable(const GMObject& obj)
{
	D(d);
	static const GMString s_overrideList[] =
	{
		L"__index",
		L"__newindex",
		L"__gc",
	};

	GM_CHECK_LUA_STACK_BALANCE(0);
	auto tableIdx = lua_gettop(L);

	bool hasIndex = false;
	GMString metatableName;
	const GMObject* metaTable = nullptr;
	getMetaTableAndName(obj, metatableName, &metaTable);
	GM_ASSERT(metaTable);

	luaL_newmetatable(L, metatableName.toStdString().c_str());
	if (!metatableName.isEmpty())
	{
		// 存在meta数据
		for (const auto& member : *metaTable->meta())
		{
			// 只为几个默认的meta写方法
			if (contains(s_overrideList, member.first) &&
				member.second.type == GMMetaMemberType::Function)
			{
				std::string name = member.first.toStdString();
				lua_pushstring(L, name.c_str());
				lua_pushcfunction(L, (GMLuaCFunction)(member.second.ptr));
				lua_rawset(L, -3);
			}
		}

	}

	lua_setmetatable(L, tableIdx);
}

bool GMLua::popTable(GMObject& obj)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	return popTable(obj, lua_gettop(L));
}

bool GMLua::popTable(GMObject& obj, GMint32 index)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;

	if (!lua_istable(L, index))
	{
		gm_error(gm_dbg_wrap("GMLua::popTable: lua object type is {0}. Table is expected."), lua_typename(L, lua_type(L, index)));
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
				{
					if (lua_isfunction(L, -1))
					{
						int r = luaL_ref(L, LUA_REGISTRYINDEX);
						*(static_cast<GMLuaReference*>(member.second.ptr)) = r;
						lua_pushnil(L); // 最开始的POP_GUARD强行pop，所以我们这里push一个nil
					}
					else
					{
						*(static_cast<GMint32*>(member.second.ptr)) = lua_tointeger(L, -1);
					}
					break;
				}
				case GMMetaMemberType::Vector2:
				{
					GMVec2& v = *static_cast<GMVec2*>(member.second.ptr);
					if (!popVector(v))
						return false;
					break;
				}
				case GMMetaMemberType::Vector3:
				{
					GMVec3& v = *static_cast<GMVec3*>(member.second.ptr);
					if (!popVector(v))
						return false;
					break;
				}
				case GMMetaMemberType::Vector4:
				{
					GMVec4& v = *static_cast<GMVec4*>(member.second.ptr);
					if (!popVector(v))
						return false;
					break;
				}
				case GMMetaMemberType::Matrix4x4:
				{
					GMMat4& mat = *static_cast<GMMat4*>(member.second.ptr);
					if (!popMatrix(mat))
						return false;
					break;
				}
				case GMMetaMemberType::Object:
				{
					GMObject* o = *static_cast<GMObject**>(member.second.ptr);
					if (o && !popTable(*o))
						return false;
					break;
				}
				case GMMetaMemberType::Pointer:
				{
					GM_STATIC_ASSERT_SIZE(GMsize_t, sizeof(void*));
					lua_Integer address = lua_tointeger(L, -1);
					*(static_cast<GMsize_t*>(member.second.ptr)) = address;
					break;
				}
				case GMMetaMemberType::Function:
					break;
				default:
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
	for (GMint32 i = 0; i < GMMat4::length(); i++)
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
	if (var.isInt() || var.isInt64())
	{
		lua_pushinteger(L, var.toInt64());
	}
	else if (var.isUInt())
	{
		lua_pushinteger(L, var.toUInt());
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
		pushNewTable(*var.toObject());
	}
	else if (var.isPointer())
	{
		GM_STATIC_ASSERT(sizeof(lua_Integer) >= sizeof(void*), "Pointer size incompatible.");
		lua_pushinteger(L, (lua_Integer)var.toPointer());
	}
	else if (var.isVec2())
	{
		pushVector(var.toVec2());
	}
	else if (var.isVec3())
	{
		pushVector(var.toVec3());
	}
	else if (var.isVec4())
	{
		pushVector(var.toVec4());
	}
	else
	{
		gm_error(gm_dbg_wrap("GMLua (push): variant type not supported"));
		GM_ASSERT(false);
	}
}

void GMLua::pop(GMint32 num)
{
	D(d);
	lua_pop(L, num);
}

void GMLua::setTable(const char* key, const GMObjectMember& value)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	lua_pushstring(L, key);

	switch (value.type)
	{
	case GMMetaMemberType::Int:
		lua_pushinteger(L, *static_cast<GMint32*>(value.ptr));
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
		break;
	}
	case GMMetaMemberType::Vector2:
	{
		GMVec2& vec2 = *static_cast<GMVec2*>(value.ptr);
		pushVector(vec2);
		GM_ASSERT(lua_istable(L, -1));
		break;
	}
	case GMMetaMemberType::Vector3:
	{
		GMVec3& vec3 = *static_cast<GMVec3*>(value.ptr);
		pushVector(vec3);
		GM_ASSERT(lua_istable(L, -1));
		break;
	}
	case GMMetaMemberType::Vector4:
	{
		GMVec4& vec4 = *static_cast<GMVec4*>(value.ptr);
		pushVector(vec4);
		GM_ASSERT(lua_istable(L, -1));
		break;
	}
	case GMMetaMemberType::Matrix4x4:
	{
		GMMat4& mat = *static_cast<GMMat4*>(value.ptr);
		pushMatrix(mat);
		GM_ASSERT(lua_istable(L, -1));
		break;
	}
	case GMMetaMemberType::Object:
	{
		GMObject* obj = *static_cast<GMObject**>(value.ptr);
		if (obj)
		{
			pushNewTable(*obj);
		}
		else
		{
			// 回滚操作，直接返回
			lua_pop(L, 1);
			return;
		}
		break;
	}
	case GMMetaMemberType::Function:
		lua_pushcfunction(L, (GMLuaCFunction)(value.ptr));
		break;
	case GMMetaMemberType::Pointer:
	{
		GM_STATIC_ASSERT(sizeof(lua_Integer) >= sizeof(void*), "Pointer size incompatible.");
		GM_STATIC_ASSERT_SIZE(GMsize_t, sizeof(void*));
		lua_Integer address = lua_tointeger(L, -1);
		lua_pushinteger(L, *static_cast<GMsize_t*>(value.ptr));
		break;
	}
	default:
		GM_ASSERT(false);
		break;
	}

	GM_ASSERT(lua_istable(L, -3));
	lua_settable(L, -3);
}

GMVariant GMLua::getTop()
{
	return get(-1);
}

GMVariant GMLua::get(GMint32 index)
{
	D(d);
	GM_CHECK_LUA_STACK_BALANCE(0);
	if (lua_isinteger(L, index))
		return static_cast<GMint32>(lua_tointeger(L, index));
	if (lua_isnumber(L, index))
		return lua_tonumber(L, index);
	if (lua_isstring(L, index))
		return GMString(lua_tostring(L, index));
	if (lua_isboolean(L, index))
		return lua_toboolean(L, index) ? true : false;
	gm_error(gm_dbg_wrap("GMLua (pop): type not supported"));
	return GMVariant();
}

GMLuaReference gm::GMLua::popFunction(bool* valid)
{
	D(d);
	if (lua_isfunction(L, -1))
	{
		GM_CHECK_LUA_STACK_BALANCE(-1);
		if (valid)
			*valid = true;
		return luaL_ref(L, LUA_REGISTRYINDEX);
	}
	else
	{
		GM_CHECK_LUA_STACK_BALANCE(0);
		if (valid)
			*valid = false;
	}
	return 0;
}
