#ifndef __GMLUA_H__
#define __GMLUA_H__
#include "common.h"
BEGIN_NS

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

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

GM_PRIVATE_OBJECT(GMLua)
{
	lua_State* luaState = nullptr;
	GMLuaExceptionHandler* exceptionHandler = nullptr;
	GMuint* ref = nullptr;
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
		double valFloat;
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

	GMLuaVariable(double d)
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

	GMLuaVariable(const GMWchar* str)
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
		ASSERT(type == GMLuaVariableType::Object);
		return valPtrObject;
	}

	~GMLuaVariable()
	{
		if (type == GMLuaVariableType::String)
			delete valPtrString;
	}
};

struct GMLuaStack
{
	GMint type;
	GMint index;
	union
	{
		bool valBool;
		const char* valStr;
		void* other;
		double valDouble;
	};
};

class GMLua : public GMObject
{
	DECLARE_PRIVATE(GMLua)

public:
	GMLua();
	~GMLua();

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
	GMLuaStack getTopStack();
	bool invoke(const char* expr);

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

private:
	void loadLibrary();
	void callExceptionHandler(GMLuaStatus state, const char* msg);
	GMLuaStatus callp(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMint nRet);
	void setTable(GMObject& obj);
	bool getTable(GMObject& obj);
	void push(const GMLuaVariable& var);
	void push(const char* name, const GMObjectMember& member);
	GMLuaVariable pop();
};

END_NS
#endif