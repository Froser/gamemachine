#ifndef __GMLUA_H__
#define __GMLUA_H__
#include <gmcommon.h>
#include <linearmath.h>

extern "C"
{
#include <lua.h>
}

BEGIN_NS

#define L (d->luaState)
#define POP_GUARD() \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : m_L(__L) {}\
	~__PopGuard() { lua_pop(m_L, 1); }		\
	lua_State* m_L;							\
} __guard(*this);

GM_INTERFACE(ILuaFunctionRegister)
{
	virtual void registerFunctions(lua_State*) = 0;
};

enum class GMLuaStates
{
	WrongType = -1,
	Ok = LUA_OK,
	RuntimeError = LUA_ERRRUN,
	SyntaxError = LUA_ERRSYNTAX,
	MemoryError = LUA_ERRMEM,
	ErrorHandlerError = LUA_ERRERR,
	GCError = LUA_ERRGCMM,
};

struct GMLuaResult
{
	GMLuaStates state;
	GMString message;
};

GM_INTERFACE(ILuaExceptionHandler)
{
	virtual void onException(GMLuaStates state, const char* msg) = 0;
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
		GMint64 valInt;
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

	GMLuaVariable(GMLuaVariable&& v) GM_NOEXCEPT
	{
		*this = std::move(v);
	}

	GMLuaVariable& operator=(GMLuaVariable&& v) GM_NOEXCEPT
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

	GMLuaVariable(GMint64 d)
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
	lua_State* luaState = nullptr;
	bool isWeakLuaStatePtr = false;
	ILuaExceptionHandler* exceptionHandler = nullptr;
	bool libraryLoaded = false;
};

class GMLua : public GMObject
{
	GM_DECLARE_PRIVATE(GMLua)

public:
	GMLua();
	GMLua(lua_State*);
	~GMLua();

public:
	GMLuaResult runFile(const char* file);
	GMLuaResult runBuffer(const GMBuffer& buffer);
	GMLuaResult runString(const GMString& string);

	void setGlobal(const char* name, const GMLuaVariable& var);
	bool setGlobal(const char* name, GMObject& obj);
	GMLuaVariable getGlobal(const char* name);
	bool getGlobal(const char* name, GMObject& obj);
	GMLuaStates call(const char* functionName, const std::initializer_list<GMLuaVariable>& args);
	GMLuaStates call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable* returns, GMint nRet);
	GMLuaStates call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMObject* returns, GMint nRet);
	bool invoke(const char* expr);

	//! 将一个对象的成员压入Lua的虚拟堆栈。
	/*!
	  在Lua C函数被调用时，使用此方法，将返回一个Lua table。
	  \param obj 待传入的对象。此对象必须要注册元对象。
	  \sa GMObject::registerMeta()
	*/
	void setTable(const GMObject& obj);

	//! 从Lua虚拟堆栈中取出一个Table，并赋值给指定对象。
	/*!
	  在Lua C函数被调用时，使用此方法，将从Lua虚拟堆栈中取出一个Table赋值给指定对象。如果操作的对象不是Table，则操作失败。
	  \param obj 待接收的对象。此对象必须要注册元对象。
	  \return 操作是否成功。
	  \sa GMObject::registerMeta()
	*/
	bool getTable(GMObject& obj);

	//! 从Lua虚拟堆栈的指定某一层堆栈中取出一个Table，并赋值给指定对象。
	/*!
	  在Lua C函数被调用时，使用此方法，将从Lua虚拟堆栈中取出一个Table赋值给指定对象。如果操作的对象不是Table，则操作失败。
	  \param obj 待接收的对象。此对象必须要注册元对象。
	  \param index 指定的堆栈索引。
	  \return 操作是否成功。
	  \sa GMObject::registerMeta()
	*/
	bool getTable(GMObject& obj, GMint index);

public:
	template <size_t _size> GMLuaStates call(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMLuaVariable(&returns)[_size])
	{
		GMLuaStates result = callp(functionName, args, _size);
		if (result == GMLuaStates::Ok)
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
		GMFloat4 f4;
		v.loadFloat4(f4);
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

			f4[key] = lua_tonumber(L, -1);
		}
		return true;
	}

	void setMatrix(const GMMat4& v)
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
	void registerLibraries();
	void callExceptionHandler(GMLuaStates state, const char* msg);
	GMLuaStates callp(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMint nRet);
	void push(const GMLuaVariable& var);
	void push(const char* name, const GMObjectMember& member);
	GMLuaVariable pop();
};

#undef L
#undef POP_GUARD
END_NS
#endif