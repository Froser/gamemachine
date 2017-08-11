#include "stdafx.h"
#include "gmlua.h"
#include "foundation/linearmath.h"
#include "gmlua_functions.h"

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
	GMLua_Vector2() = default;

	GMLua_Vector2(GMfloat x, GMfloat y)
	{
		D(d);
		d->x = x;
		d->y = y;
	}

	GMfloat x() const { D(d); return d->x; }
	GMfloat y() const { D(d); return d->y; }
};

GM_PRIVATE_OBJECT_FROM(GMLua_Vector3, GMLua_Vector2)
{
	GMfloat z;
};

class GMLua_Vector3 : public GMLua_Vector2
{
	DECLARE_PRIVATE(GMLua_Vector3)

	GM_BEGIN_META_MAP_FROM(GMLua_Vector2)
		GM_META(z, GMMetaMemberType::Float)
	GM_END_META_MAP

public:
	GMLua_Vector3() = default;

	GMLua_Vector3(GMfloat x, GMfloat y, GMfloat z)
		: GMLua_Vector2(x, y)
	{
		D(d);
		d->z = z;
	}

	GMLua_Vector3(const linear_math::Vector3& v)
	{
		D(d);
		D_BASE(db, GMLua_Vector2);
		db->x = v[0];
		db->y = v[1];
		d->z = v[2];
	}

	GMfloat z() const { D(d); return d->z; }
};

GM_PRIVATE_OBJECT_FROM(GMLua_Vector4, GMLua_Vector3)
{
	GMfloat w;
};

class GMLua_Vector4 : public GMLua_Vector3
{
	DECLARE_PRIVATE(GMLua_Vector4)

	GM_BEGIN_META_MAP_FROM(GMLua_Vector3)
		GM_META(w, GMMetaMemberType::Float)
	GM_END_META_MAP

public:
	GMLua_Vector4() = default;

	GMLua_Vector4(GMfloat x, GMfloat y, GMfloat z, GMfloat w)
		: GMLua_Vector3(x, y, z)
	{
		D(d);
		d->w = w;
	}
	
	GMLua_Vector4& operator=(const GMLua_Vector4& rhs)
	{
		D(d);
		d->x = rhs.x();
		d->y = rhs.y();
		d->z = rhs.z();
		d->w = rhs.w();
		return *this;
	}

	GMfloat w() const { D(d); return d->w; }
};

GM_PRIVATE_OBJECT(GMLua_Matrix4x4)
{
	linear_math::Vector4 r1, r2, r3, r4;
};

class GMLua_Matrix4x4 : public GMObject
{
	DECLARE_PRIVATE(GMLua_Matrix4x4)

	GM_BEGIN_META_MAP
		GM_META(r1, GMMetaMemberType::Vector4)
		GM_META(r2, GMMetaMemberType::Vector4)
		GM_META(r3, GMMetaMemberType::Vector4)
		GM_META(r4, GMMetaMemberType::Vector4)
	GM_END_META_MAP

public:
	GMLua_Matrix4x4() = default;
	GMLua_Matrix4x4(const linear_math::Vector4& r1, const linear_math::Vector4& r2, const linear_math::Vector4& r3, const linear_math::Vector4& r4)
	{
		D(d);
		d->r1 = r1;
		d->r2 = r2;
		d->r3 = r3;
		d->r4 = r4;
	}

	linear_math::Vector4& r1() { D(d); return d->r1; }
	linear_math::Vector4& r2() { D(d); return d->r2; }
	linear_math::Vector4& r3() { D(d); return d->r3; }
	linear_math::Vector4& r4() { D(d); return d->r4; }
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
	loadLibrary();
	return (GMLuaStatus)(luaL_loadfile(L, file) || lua_pcall(L, 0, LUA_MULTRET, 0));
}

GMLuaStatus GMLua::loadBuffer(const GMBuffer& buffer)
{
	D(d);
	ASSERT(L);
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
	return result;
}

GMLuaStatus GMLua::call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable* returns, GMint nRet)
{
	GMLuaStatus result = call(functionName, args);
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
	GMLuaStatus result = call(functionName, args);
	if (result == GMLuaStatus::OK)
	{
		for (GMint i = 0; i < nRet; i++)
		{
			if (getTable(returns[i]))
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
	luaL_requiref(L, "gmcore", register_core, 1);
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

bool GMLua::getTable(GMObject& obj)
{
	D(d);
	const GMMeta* meta = obj.meta();
	if (!meta)
		return false;

	GMint index = lua_gettop(L);
	ASSERT(lua_istable(L, index));
	lua_pushnil(L);
	while (lua_next(L, index))
	{
		ASSERT(lua_isstring(L, -2));
		const char* key = lua_tostring(L, -2);
		for (const auto member : *meta)
		{
			if (strEqual(member.first, key))
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
						GMLua_Vector2 v;
						if (!getTable(v))
							return false;
						linear_math::Vector2* vec2 = static_cast<linear_math::Vector2*>(member.second.ptr);
						(*vec2)[0] = v.x();
						(*vec2)[1] = v.y();
					}
					break;
				case GMMetaMemberType::Vector3:
					{
						GMLua_Vector3 v;
						if (!getTable(v))
							return false;
						linear_math::Vector3* vec3 = static_cast<linear_math::Vector3*>(member.second.ptr);
						(*vec3)[0] = v.x();
						(*vec3)[1] = v.y();
						(*vec3)[2] = v.z();
					}
					break;
				case GMMetaMemberType::Vector4:
					{
						GMLua_Vector4 v;
						if (!getTable(v))
							return false;
						linear_math::Vector4* vec4 = static_cast<linear_math::Vector4*>(member.second.ptr);
						(*vec4)[0] = v.x();
						(*vec4)[1] = v.y();
						(*vec4)[2] = v.z();
						(*vec4)[3] = v.w();
					}
					break;
				case GMMetaMemberType::Matrix4x4:
					{
						GMLua_Matrix4x4 mat;
						if (!getTable(mat))
							return false;
						linear_math::Matrix4x4& mat4 = *static_cast<linear_math::Matrix4x4*>(member.second.ptr);
						mat4[0] = mat.r1();
						mat4[1] = mat.r2();
						mat4[2] = mat.r3();
						mat4[3] = mat.r4();
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
					ASSERT(false);
					break;
				}
			}
		}

		lua_pop(L, 1);
	}
	return true;
}

void GMLua::push(const GMLuaVariable& var)
{
	D(d);
	switch (var.type)
	{
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
			ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Vector3:
		{
			linear_math::Vector3& vec3 = *static_cast<linear_math::Vector3*>(member.ptr);
			GMLua_Vector3 lua_vec3(vec3[0], vec3[1], vec3[2]);
			setTable(lua_vec3);
			ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Vector4:
		{
			linear_math::Vector4& vec4 = *static_cast<linear_math::Vector4*>(member.ptr);
			GMLua_Vector4 lua_vec4(vec4[0], vec4[1], vec4[2], vec4[3]);
			setTable(lua_vec4);
			ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Matrix4x4:
		{
			linear_math::Matrix4x4& mat= *static_cast<linear_math::Matrix4x4*>(member.ptr);
			GMLua_Matrix4x4 lua_vec4(mat[0], mat[1], mat[2], mat[3]);
			setTable(lua_vec4);
			ASSERT(lua_istable(L, -1));
		}
		break;
	case GMMetaMemberType::Object:
		setTable(*static_cast<GMObject*>(member.ptr));
		break;
	default:
		ASSERT(false);
		break;
	}

	ASSERT(lua_istable(L, -3));
	lua_settable(L, -3);
}

GMLuaVariable GMLua::pop()
{
	D(d);
	POP_GUARD(d);
	if (lua_isinteger(L, -1))
		return lua_tointeger(L, -1);
	if (lua_isnumber(L, -1))
		return lua_tonumber(L, -1);
	if (lua_isstring(L, -1))
		return GMString(lua_tostring(L, -1));
	if (lua_isboolean(L, -1))
		return lua_toboolean(L, -1);
	ASSERT(false);
	return 0;
}