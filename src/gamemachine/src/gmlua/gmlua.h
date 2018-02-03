#ifndef __GMLUA_H__
#define __GMLUA_H__
#include <gmcommon.h>
#include <linearmath.h>

BEGIN_NS

extern "C"
{
#include <lua.h>
}

#define L (d->luaState)
#define POP_GUARD() \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : m_L(__L) {}\
	~__PopGuard() { lua_pop(m_L, 1); }		\
	lua_State* m_L;							\
} __guard(*this);

enum class GMLuaStatus
{
	WRONG_TYPE = -1,
	OK = LUA_OK,
	ERRRUN = LUA_ERRRUN,
	ERRMEM = LUA_ERRMEM,
	ERRERR = LUA_ERRERR,
	ERRGCMM = LUA_ERRGCMM,
};

GM_INTERFACE(GMLuaExceptionHandler)
{
	virtual void onException(GMLuaStatus state, const char* msg) = 0;
};

enum class GMLuaVariableType
{
	Int,
	Number,
	String,
	Boolean,
	Object
};

struct GMLuaVariable
{
	GMLuaVariableType type;
	union
	{
		GMString* valPtrString;
		GMObject* valPtrObject;
		GMLargeInteger valInt;
		bool valBoolean;
		GMfloat valFloat;
	};

	GMLuaVariable(const GMLuaVariable& v)
	{
		*this = v;
	}

	GMLuaVariable& operator=(const GMLuaVariable& v)
	{
		if (&v == this)
			return *this;

		if (v.type == GMLuaVariableType::String)
		{
			type = v.type;
			valPtrString = new GMString(*v.valPtrString);
		}
		else
		{
			memcpy_s(this, sizeof(*this), &v, sizeof(v));
		}
		return *this;
	}

	GMLuaVariable(GMLuaVariable&& v) noexcept
	{
		*this = std::move(v);
	}

	GMLuaVariable& operator=(GMLuaVariable&& v) noexcept
	{
		if (&v == this)
			return *this;

		if (v.type == GMLuaVariableType::String)
		{
			delete valPtrString;
			type = v.type;
			valPtrString = v.valPtrString;
			v.valPtrString = nullptr;
		}
		else if (v.type == GMLuaVariableType::Object)
		{
			type = v.type;
			valPtrObject = v.valPtrObject;
			v.valPtrObject = nullptr;
		}
		else
		{
			memcpy_s(this, sizeof(*this), &v, sizeof(v));
		}

		return *this;
	}

	GMLuaVariable()
		: type(GMLuaVariableType::Int)
		, valInt(0)
	{
	}

	GMLuaVariable(GMLargeInteger d)
		: type(GMLuaVariableType::Int)
		, valInt(d)
	{
	}

	GMLuaVariable(GMint d)
		: type(GMLuaVariableType::Int)
		, valInt(d)
	{
	}

	GMLuaVariable(GMfloat d)
		: type(GMLuaVariableType::Number)
		, valFloat(d)
	{
	}

	explicit GMLuaVariable(bool d)
		: type(GMLuaVariableType::Boolean)
		, valBoolean(d)
	{
	}

	GMLuaVariable(const char* str)
		: type(GMLuaVariableType::String)
		, valPtrString(new GMString(str))
	{
	}

	GMLuaVariable(const GMwchar* str)
		: type(GMLuaVariableType::String)
		, valPtrString(new GMString(str))
	{
	}

	GMLuaVariable(AUTORELEASE const GMString& str)
		: type(GMLuaVariableType::String)
		, valPtrString(new GMString(str))
	{
	}

	GMLuaVariable(GMObject& obj)
		: type(GMLuaVariableType::Object)
		, valPtrObject(&obj)
	{
	}

	operator GMObject*()
	{
		GM_ASSERT(type == GMLuaVariableType::Object);
		return valPtrObject;
	}

	~GMLuaVariable()
	{
		if (type == GMLuaVariableType::String)
			delete valPtrString;
	}
};

GM_PRIVATE_OBJECT(GMLua)
{
	bool weakRef = false;
	lua_State* luaState = nullptr;
	GMLuaExceptionHandler* exceptionHandler = nullptr;
	GMuint* ref = nullptr;
};

class GMLua
{
	DECLARE_PRIVATE_NGO(GMLua)

public:
	GMLua();
	~GMLua();
	GMLua(lua_State* l);
	GMLua(const GMLua& lua);
	GMLua(GMLua&& lua) noexcept;
	GMLua& operator= (const GMLua& state);
	GMLua& operator= (GMLua&& state) noexcept;

public:
	GMLuaStatus loadFile(const char* file);
	GMLuaStatus loadBuffer(const GMBuffer& buffer);

	void setGlobal(const char* name, const GMLuaVariable& var);
	bool setGlobal(const char* name, GMObject& obj);
	GMLuaVariable getGlobal(const char* name);
	bool getGlobal(const char* name, GMObject& obj);
	GMLuaStatus call(const char* functionName, const std::initializer_list<GMLuaVariable>& args);
	GMLuaStatus call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable* returns, GMint nRet);
	GMLuaStatus call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMObject* returns, GMint nRet);
	bool invoke(const char* expr);
	void setTable(GMObject& obj);
	bool getTable(GMObject& obj);
	bool getTable(GMObject& obj, GMint index);

public:
	template <size_t _size> GMLuaStatus call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable(&returns)[_size])
	{
		GMLuaStatus result = callp(functionName, args, _size);
		if (result == GMLuaStatus::OK)
		{
			for (GMint i = 0; i < _size; i++)
			{
				returns[i] = pop();
			}
		}
		return result;
	}

	operator lua_State*()
	{
		D(d);
		return d->luaState;
	}


	template <typename T>
	void setVector(const T& v)
	{
		D(d);
		lua_newtable(L);
		GMFloat4 f4;
		v.loadFloat4(f4);
		for (GMint i = 0; i < T::length(); i++)
		{
			lua_pushnumber(L, i);
			lua_pushnumber(L, f4[i]);
			GM_ASSERT(lua_istable(L, -3));
			lua_settable(L, -3);
		}
	}

	template <typename T>
	bool getVector(T& v)
	{
		D(d);
		GMint index = lua_gettop(L);
		return getVector(v, index);
	}

	template <typename T>
	bool getVector(T& v, GMint index)
	{
		D(d);
		GM_ASSERT(lua_istable(L, index));
		lua_pushnil(L);
		while (lua_next(L, index))
		{
			POP_GUARD();
			if (!lua_isinteger(L, -2))
				return false;

			GMint key = lua_tointeger(L, -2);
			if (!lua_isnumber(L, -1))
				return false;

			v[key] = lua_tonumber(L, -1);
		}
		return true;
	}

	template <typename T>
	void setMatrix(const T& v)
	{
		D(d);
		lua_newtable(L);
		for (GMint i = 0; i < T::length(); i++)
		{
			lua_pushnumber(L, i);
			setVector(v[i]);
			GM_ASSERT(lua_istable(L, -3));
			lua_settable(L, -3);
		}
	}

	template <typename T>
	bool getMatrix(T& v)
	{
		D(d);
		GMint index = lua_gettop(L);
		return getMatrix(v, index);
	}

	template <typename T>
	bool getMatrix(T& v, GMint index)
	{
		D(d);
		GM_ASSERT(lua_istable(L, index));
		lua_pushnil(L);
		while (lua_next(L, index))
		{
			POP_GUARD();
			if (!lua_isinteger(L, -2))
				return false;

			GMint key = lua_tointeger(L, -2);
			bool isVector = getVector(v[key]);
			if (!isVector)
				return false;
		}
		return true;
	}

private:
	void loadLibrary();
	void callExceptionHandler(GMLuaStatus state, const char* msg);
	GMLuaStatus callp(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMint nRet);
	void push(const GMLuaVariable& var);
	void push(const char* name, const GMObjectMember& member);
	GMLuaVariable pop();
};

#undef L
#undef POP_GUARD
END_NS
#endif