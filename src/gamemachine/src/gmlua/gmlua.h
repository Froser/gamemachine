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
#define POP_GUARD() \
struct __PopGuard							\
{											\
	__PopGuard(lua_State* __L) : m_L(__L) {}\
	~__PopGuard() { lua_pop(m_L, 1); }		\
	lua_State* m_L;							\
} __guard(this->getLuaCoreState());

#define GM_LUA_DECLARATIONS(name) { #name, name }

typedef luaL_Reg GMLuaReg;
typedef lua_State GMLuaCoreState;
typedef lua_CFunction GMLuaCFunction;
typedef int GMLuaReference;

class GMLua;
namespace luaapi
{
	typedef GMint32 GMFunctionReturn;
	struct GMArgumentHelper;
	struct GMReturnValues;
}

struct GM_EXPORT GMLuaFunctionRegister
{
	virtual void registerFunctions(GMLua*) = 0;

protected:
	void setRegisterFunction(GMLua *l, const GMString& modname, GMLuaCFunction openf, bool isGlobal);
	
protected:
	static void newLibrary(GMLuaCoreState *l, const GMLuaReg* functions);
};

// 检查栈是否平衡
#if GM_DEBUG
#	define GM_CHECK_LUA_STACK_BALANCE(offset) gm::GMLuaStackBalanceCheck __balanceGuard(L, offset);
#	define GM_CHECK_LUA_STACK_BALANCE_(l, offset) gm::GMLuaStackBalanceCheck __balanceGuard(l, offset);

class GMLuaStackBalanceCheck
{
public:
	GMLuaStackBalanceCheck(GMLuaCoreState* l, GMint32 offset)
		: m_L(l)
		, m_offset(offset)
	{
		m_stack = lua_gettop(m_L);
	}

	~GMLuaStackBalanceCheck()
	{
#if GM_DEBUG
		auto currentTop = lua_gettop(m_L);
		GM_ASSERT(currentTop == m_stack + m_offset);
#endif
	}

private:
	GMint32 m_stack;
	GMint32 m_offset;
	GMLuaCoreState* m_L;
};

#else
#	define GM_CHECK_LUA_STACK_BALANCE(offset)
#	define GM_CHECK_LUA_STACK_BALANCE_(l, offset)
#endif

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
	friend struct luaapi::GMArgumentHelper;
	friend struct GMReturnValues;

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

// 针对堆栈的操作，提供给友元 Deprecating
private:
	//! 将一个对象的成员压入Lua的虚拟堆栈。
	/*!
	  在Lua C函数被调用时，使用此方法，将返回一个Lua table。
	  \param obj 待传入的对象。此对象必须要注册元对象。
	  \sa GMObject::registerMeta()
	*/
	void pushNewTable(const GMObject& obj, bool setMetatable = true);

	//! 从Lua虚拟堆栈中取出一个Table，并赋值给指定对象。
	/*!
	  在Lua C函数被调用时，使用此方法，将从Lua虚拟堆栈中取出一个Table赋值给指定对象。如果操作的对象不是Table，则操作失败。
	  \param obj 待接收的对象。此对象必须要注册元对象。
	  \return 操作是否成功。
	  \sa GMObject::registerMeta()
	*/
	bool popTable(GMObject& obj);

	//! 从Lua虚拟堆栈的指定某一层堆栈中取出一个Table，并赋值给指定对象。
	/*!
	  在Lua C函数被调用时，使用此方法，将从Lua虚拟堆栈中取出一个Table赋值给指定对象。如果操作的对象不是Table，则操作失败。
	  \param obj 待接收的对象。此对象必须要注册元对象。
	  \param index 指定的堆栈索引。
	  \return 操作是否成功。
	  \sa GMObject::registerMeta()
	*/
	bool popTable(GMObject& obj, GMint32 index);
	bool popVector(GMVec2& v);
	bool popVector(GMVec3& v);
	bool popVector(GMVec4& v);
	bool popMatrix(GMMat4& v);

	//! 返回Lua的栈顶的变量。
	/*!
	  在调用此方法前，必须确认Lua栈顶是非空的。它将返回Lua栈顶的值但不弹出它，并将其转化为一个GMVariant。需要注意的是，它并不能识别Vec2, Vec3, Vec4和Mat4，因为在Lua中，它们都是表(table)。
	  \return Lua栈顶转化为GMVariant后的值。
	*/
	GMVariant getTop();
	GMVariant get(GMint32 index);
	GMLuaReference popFunction(bool* valid = nullptr);
	void pop(GMint32 num);
	void setEachMetaMember(const GMObject& obj);

public:
	GMLuaCoreState* getLuaCoreState() GM_NOEXCEPT;

public:
	static GMLuaRuntime* getRuntime(GMLuaCoreState*);

private:
	void loadLibrary();
	GMLuaResult pcall(const std::initializer_list<GMVariant>& args, GMint32 nRet);
	GMLuaResult pcallreturn(GMLuaResult, GMVariant* returns, GMint32 nRet);
	void setTable(const char* key, const GMObjectMember& value);
	void setMetatable(const GMObject& obj);
};

#undef L
#undef POP_GUARD

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

#define GM_LUA_FUNC(FuncName) gm::luaapi::GMFunctionReturn FuncName(GMLuaCoreState* L)
#define GM_LUA_DEFAULT_NEW_IMPL(New, FuncName, Proxy, Real) GM_LUA_FUNC(New)				\
{																							\
	static const GMString s_invoker = FuncName;												\
	GM_LUA_CHECK_ARG_COUNT(L, 0,FuncName);													\
	Proxy proxy(L, new Real());																\
	return gm::GMReturnValues(L, GMVariant(proxy));									\
}

#define GM_LUA_NEW_IMPL_ARG(New, FuncName, Proxy, Real, Arg0ProxyType) GM_LUA_FUNC(New)		\
{																							\
	static const GMString s_invoker = FuncName;												\
	GM_LUA_CHECK_ARG_COUNT(L, 1, FuncName);													\
	gm::luaapi::Arg0ProxyType arg0(L);														\
	gm::luaapi::GMArgumentHelper::popArgumentAsObject(L, arg0, s_invoker);					\
	Proxy proxy(L, new Real(arg0.get()));													\
	return gm::GMReturnValues(L, GMVariant(proxy));									\
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
	{																										\
		static const GMString s_invoker = #proxyClass ".__index";											\
		proxyClass self(L, nullptr);																		\
		GMVariant key = gm::luaapi::GMArgumentHelper::peekArgument(L, 2, s_invoker); /*key*/				\
		HashMap<GMString, std::function<gm::GMReturnValues()>, GMStringHashFunctor> __s_indexMap;	\
		if (__s_indexMap.empty())																			\
		{

#define GM_LUA_PROPERTY_PROXY_GETTER(targetProxy, name, memberName) \
		{ __s_indexMap[#memberName] = [&]() {													\
			gm::luaapi::GMArgumentHelper::popArgument(L, s_invoker); /*key*/					\
			gm::luaapi::GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/		\
			targetProxy proxy(L, nullptr);														\
			proxy.set(&self->get##name());														\
			return gm::GMReturnValues(L, proxy); }; }

#define GM_LUA_PROPERTY_GETTER(name, memberName) \
		{ __s_indexMap[#memberName] = [&]() {													\
			GMArgumentHelper::popArgument(L, s_invoker); /*key*/								\
			GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/					\
			return gm::GMReturnValues(L, (self->get##name())); }; }

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

#define GM_LUA_PROPERTY_SETTER(name, memberName, propertyType) \
		{ __s_indexMap[#memberName] = [&]() {																									\
			if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_INT)) ||			\
				GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_INT64)) ||		\
				GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_UINT)) ||		\
				GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_FLOAT)))			\
			{																																	\
				auto value = gm::luaapi::GMArgumentHelper::popArgument(L, s_invoker); /*value*/													\
				gm::luaapi::GMArgumentHelper::popArgument(L, s_invoker); /*key*/																\
				gm::luaapi::GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/													\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_VEC2)))		\
			{																																	\
				GMVariant value = gm::luaapi::GMArgumentHelper::popArgumentAsVec2(L, s_invoker); /*value*/										\
				gm::luaapi::GMArgumentHelper::popArgument(L, s_invoker); /*key*/																\
				gm::luaapi::GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/													\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_VEC3)))		\
			{																																	\
				GMVariant value = gm::luaapi::GMArgumentHelper::popArgumentAsVec3(L, s_invoker); /*value*/										\
				gm::luaapi::GMArgumentHelper::popArgument(L, s_invoker); /*key*/																\
				gm::luaapi::GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/													\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_VEC4)))		\
			{																																	\
				GMVariant value = gm::luaapi::GMArgumentHelper::popArgumentAsVec4(L, s_invoker); /*value*/										\
				gm::luaapi::GMArgumentHelper::popArgument(L, s_invoker); /*key*/																\
				gm::luaapi::GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/													\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_MAT4)))		\
			{																																	\
				GMVariant value = gm::luaapi::GMArgumentHelper::popArgumentAsMat4(L, s_invoker); /*value*/										\
				gm::luaapi::GMArgumentHelper::popArgument(L, s_invoker); /*key*/																\
				gm::luaapi::GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/													\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else																																\
			{																																	\
				gm_error(gm_dbg_wrap("wrong lua property type."));																				\
			}																																	\
			return gm::GMReturnValues();																								\
		}; }

#define GM_LUA_END_PROPERTY() \
		}																						\
		if (key.isString())																		\
		{																						\
			auto itemIter = __s_indexMap.find(key.toString());									\
			if (itemIter != __s_indexMap.end())													\
				if (itemIter->second)															\
					return itemIter->second();													\
		}																						\
	}																							\
	return gm::GMReturnValues();

END_NS

#include "gmlua_helper.h"

#endif