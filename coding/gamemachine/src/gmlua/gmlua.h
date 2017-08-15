#ifndef __GMLUA_H__
#define __GMLUA_H__
#include "common.h"
#include "foundation/linearmath.h"

BEGIN_NS

extern "C"
{
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

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

	operator linear_math::Vector4()
	{
		return linear_math::Vector4(x(), y(), z(), w());
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

	GMLua_Matrix4x4(const linear_math::Matrix4x4& mat)
		: GMLua_Matrix4x4(mat[0], mat[1], mat[2], mat[3])
	{
	}

	GMLua_Matrix4x4(const linear_math::Vector4& r1, const linear_math::Vector4& r2, const linear_math::Vector4& r3, const linear_math::Vector4& r4)
	{
		D(d);
		d->r1 = r1;
		d->r2 = r2;
		d->r3 = r3;
		d->r4 = r4;
	}

	operator linear_math::Matrix4x4()
	{
		return linear_math::Matrix4x4(r1(), r2(), r3(), r4());
	}

	linear_math::Vector4& r1() { D(d); return d->r1; }
	linear_math::Vector4& r2() { D(d); return d->r2; }
	linear_math::Vector4& r3() { D(d); return d->r3; }
	linear_math::Vector4& r4() { D(d); return d->r4; }
};

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

GM_PRIVATE_OBJECT(GMLua)
{
	bool weakRef = false;
	lua_State* luaState = nullptr;
	GMLuaExceptionHandler* exceptionHandler = nullptr;
	GMuint* ref = nullptr;
};

class GMLua : public GMObject
{
	DECLARE_PRIVATE(GMLua)

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
	GMLuaStack getTopStack();
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

private:
	void loadLibrary();
	void callExceptionHandler(GMLuaStatus state, const char* msg);
	GMLuaStatus callp(const char* functionName, const std::initializer_list<GMLuaVariable>& args, GMint nRet);
	void push(const GMLuaVariable& var);
	void push(const char* name, const GMObjectMember& member);
	GMLuaVariable pop();
};

END_NS
#endif