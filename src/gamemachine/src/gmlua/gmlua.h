#ifndef __GMLUA_H__
#define __GMLUA_H__
#include <gmcommon.h>
#include <linearmath.h>
#include "gmluaruntime.h"

extern "C"
{
#include <lua.h>
}

BEGIN_NS

#define L (d->luaState)
#define GM_LUA_DECLARATIONS(name) { #name, name }

typedef luaL_Reg GMLuaReg;
typedef lua_State GMLuaCoreState;
typedef lua_CFunction GMLuaCFunction;
typedef int GMLuaReference;
typedef int GMFunctionReturn;

class GMLua;
struct GM_EXPORT GMLuaFunctionRegister
{
	virtual void registerFunctions(GMLua*) = 0;

protected:
	void setRegisterFunction(GMLua *l, const GMString& modname, GMLuaCFunction openf, bool isGlobal);
	
protected:
	static void newLibrary(GMLuaCoreState *l, const GMLuaReg* functions);
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

GM_PRIVATE_CLASS(GMLua);
class GM_EXPORT GMLua
{
	GM_DECLARE_PRIVATE(GMLua)
	GM_DISABLE_COPY_ASSIGN(GMLua)

public:
	GMLua();
	GMLua(GMLuaCoreState*);
	~GMLua();

public:
	GMLuaResult runFile(const char* file);
	GMLuaResult runBuffer(const GMBuffer& buffer);

	//! 调用一个表达式。
	/*!
	在当前上下文中，调用一个表达式语句。
	\param expr 所调用的表达式。
	\return 是否调用成功。
	*/
	GMLuaResult runString(const GMString& string);

	//! 将一个变量设置进Lua的全局环境。
	/*!
	  \param name Lua全局环境的变量名。
	  \param var 待设置的变量。
	*/
	void setToGlobal(const char* name, const GMVariant& var);

	//! 通过变量名获取一个全局变量。
	/*!
	  所获取的变量将会存入GMVariant结构。需要注意的是，它只能获取标量(int64, float, boolean, string)，无法获取向量、矩阵，认为它们在Lua中是以表的形式存在。<BR>
	  如果要处理向量、矩阵，请使用getGlobal的另外一个版本。所获取的整形变量为int64类型，如果遇到无法获取的情况，将返回一个Unknown类型的GMVariant。<BR>
	  如果obj不为空，则待获取的变量一定要是个表结构，它将寻找能够精确匹配上GMObject的Lua表对象，并将表中的值赋予GMObject。如果能精确匹配，返回为true的GMVariant，否则返回为false的GMVariant。
	  \param name Lua全局对象名称。
	  \param obj 需要赋值的对象。
	  \return 返回全局对象或者是否匹配到对象。
	*/
	GMVariant getFromGlobal(const char* name, GMObject* obj = nullptr);

	//! 通过函数名，在受保护模式下调用一个Lua函数。
	/*!
	  \param functionName 函数名。函数必须在全局范围内能找到。
	  \param args 调用参数。
	  \param returns 函数返回值。
	  \param nRet 函数返回个数。
	  \return 函数调用结果。
	*/
	GMLuaResult protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args = {}, GMVariant* returns = nullptr, GMint32 nRet = 0);
	
	//! 通过一个引用，在受保护模式下调用一个Lua函数。
	/*!
	  \param functionName 函数名。函数必须在全局范围内能找到。
	  \param args 调用参数。
	  \param returns 函数返回值。
	  \param nRet 函数返回个数。
	  \return 函数调用结果。
	*/
	GMLuaResult protectedCall(GMLuaReference funcRef, const std::initializer_list<GMVariant>& args = {}, GMVariant* returns = nullptr, GMint32 nRet = 0);

	//! 释放一个引用。
	/*!
	  \param ref Lua引用。
	*/
	void freeReference(GMLuaReference ref);

	//! 获取Lua状态机。
	GMLuaCoreState* getLuaCoreState() GM_NOEXCEPT;

public:
	static GMLuaRuntime* getRuntime(GMLuaCoreState*);

private:
	void loadLibrary();
	GMLuaResult pcall(const std::initializer_list<GMVariant>& args, GMint32 nRet);
	GMLuaResult pcallreturn(GMLuaResult, GMVariant* returns, GMint32 nRet);
};

#undef L

#define GM_LUA_REGISTER(MetaName) \
	class MetaName : public GMLuaFunctionRegister				\
	{															\
	public:														\
		virtual void registerFunctions(GMLua* L) override;		\
																\
	private:													\
		static int regCallback(GMLuaCoreState *L);				\
		static const char* Name;								\
	};

#define GM_LUA_REGISTER_IMPL(MetaName, ObjectName, MetaFunctions) \
	const char* MetaName::Name = ObjectName;					\
	void MetaName::registerFunctions(GMLua* L)					\
	{															\
		setRegisterFunction(L, Name, regCallback, true);		\
	}															\
	int MetaName::regCallback(GMLuaCoreState *L)				\
	{															\
		newLibrary(L, MetaFunctions);							\
		return 1;												\
	}

#define GM_LUA_FUNC(FuncName) gm::GMFunctionReturn FuncName(GMLuaCoreState* L)
#define GM_LUA_DEFAULT_NEW_IMPL(New, FuncName, Proxy, Real) GM_LUA_FUNC(New)				\
{																							\
	gm::GMLuaArguments(L, FuncName);														\
	Proxy proxy(L, new Real());																\
	return gm::GMReturnValues(L, GMVariant(proxy));											\
}

#define GM_LUA_NEW_IMPL_ARG(New, FuncName, Proxy, Real, Arg0ProxyType) GM_LUA_FUNC(New)		\
{																							\
	gm::GMLuaArguments args(L, FuncName, { GMMetaMemberType::Object } );					\
	gm::luaapi::Arg0ProxyType arg0(L);														\
	args.getArgument(0, &arg0);																\
	Proxy proxy(L, new Real(arg0.get()));													\
	return gm::GMReturnValues(L, GMVariant(proxy));											\
}

#define GM_LUA_PROXY_IMPL(Proxy, FuncName) gm::GMint32 GM_PRIVATE_NAME(Proxy)::FuncName(GMLuaCoreState* L)
#define GM_LUA_PROXY_FUNC(FuncName) GM_META_METHOD gm::GMint32 FuncName(GMLuaCoreState*);

// lua类成员函数相关的宏
// 以下宏需要手动引用gmlua_functions.h

#define GM_LUA_PROXY_OBJ(realType, baseName) \
	public:																			\
		realType* get() const { return gm_cast<realType*>(baseName::get()); }		\
		realType* operator->() const { return get(); }

// Lua indexer，用于模拟GMObject的属性
#define GM_LUA_BEGIN_PROPERTY(proxyClass) \
	{																											\
		using Callback = gm::GMReturnValues(GMLuaCoreState*, proxyClass&, const GMLuaArguments&);				\
		using FunctionMap = HashMap<GMString, std::function<Callback>, GMStringHashFunctor>;					\
		using ProxyClass = proxyClass;																			\
		ProxyClass self(L, nullptr);																			\
		GMLuaArguments args(L, #proxyClass ".__index", { GMMetaMemberType::Object, GMMetaMemberType::String });	\
		args.getArgument(0, &self);																				\
		GMString key = args.getArgument(1).toString();															\
		static FunctionMap __s_indexMap;																		\
		static std::once_flag __once_flag;																		\
		std::call_once(__once_flag, [](FunctionMap& indexMap) {

#define GM_LUA_PROPERTY_PROXY_GETTER(targetProxy, name, key)										\
			indexMap[#key] = [](GMLuaCoreState* L, ProxyClass& m, const GMLuaArguments& args) {		\
				targetProxy proxy(L, nullptr);														\
				proxy.set(&m->get##name());															\
				return gm::GMReturnValues(L, proxy);												\
			};

#define GM_LUA_PROPERTY_GETTER(name, key)	\
			indexMap[#key] = [](GMLuaCoreState* L, ProxyClass& m, const GMLuaArguments& args) {		\
				return gm::GMReturnValues(L, m->get##name());										\
			};

#define GM_LUA_PROPERTY_TYPE_INT toInt
#define GM_LUA_PROPERTY_TYPE_INT64 toInt64
#define GM_LUA_PROPERTY_TYPE_UINT toInt
#define GM_LUA_PROPERTY_TYPE_FLOAT toFloat
#define GM_LUA_PROPERTY_TYPE_VEC2 toVec2
#define GM_LUA_PROPERTY_TYPE_VEC3 toVec3
#define GM_LUA_PROPERTY_TYPE_VEC4 toVec4
#define GM_LUA_PROPERTY_TYPE_MAT4 toMat4
#define GM_LUA_PROPERTY_TYPE_NAME(X) GM_LUA_PROPERTY_TYPE_NAME_DUMMY(X)
#define GM_LUA_PROPERTY_TYPE_NAME_DUMMY(X) #X

#define GM_LUA_PROPERTY_SETTER(name, key, propertyType) \
	indexMap[#key] = [](GMLuaCoreState* L, ProxyClass& m, const GMLuaArguments& args) {	\
	GMVariant value = args.getArgument(1);												\
	m->set##name(value.propertyType());													\
	return gm::GMReturnValues();														\
	};

#define GM_LUA_END_PROPERTY()						\
		}, __s_indexMap);							\
		auto iter = __s_indexMap.find(key);			\
		if (iter != __s_indexMap.end())				\
		{											\
			return iter->second(L, self, args);		\
		}											\
		else										\
		{											\
			return gm::GMReturnValues();			\
		}											\
	}

END_NS

#include "gmlua_helper.h"

#endif