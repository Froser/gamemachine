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

struct GMLuaFunctionRegister
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

GM_PRIVATE_OBJECT(GMLua)
{
	GMLuaCoreState* luaState = nullptr;
	bool isWeakLuaStatePtr = false;
	bool libraryLoaded = false;
};

class GM_EXPORT GMLua : public GMObject
{
	GM_DECLARE_PRIVATE(GMLua)
	friend struct luaapi::GMArgumentHelper;
	friend struct luaapi::GMReturnValues;

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

	//! 将一个变量设置进Lua的全局环境。
	/*!
	  此变量在Lua中为一个表(table)类型，里面的键、值由GMObject中的元对象决定。
	  \param name Lua全局环境的变量名。
	  \param obj 待设置的变量。
	*/
	bool setToGlobal(const char* name, GMObject& obj);

	//! 通过变量名获取一个全局变量。
	/*!
	  所获取的变量将会存入GMVariant结构。需要注意的是，它只能获取标量(int64, float, boolean, string)，无法获取向量、矩阵，认为它们在Lua中是以表的形式存在。<BR>
	  如果要处理向量、矩阵，请使用getGlobal的另外一个版本。所获取的整形变量为int64类型，如果遇到无法获取的情况，将返回一个Unknown类型的GMVariant。
	  \param name Lua全局对象名称。
	*/
	GMVariant getFromGlobal(const char* name);

	//! 通过变量名获取一个全局变量。
	/*!
	  所获取的变量一定要是个表结构，它将寻找能够精确匹配上GMObject的Lua表对象，并将表中的值赋予GMObject。如果能精确匹配，返回true，否则返回false。
	  \param name Lua全局对象名称。
	  \param obj 需要赋值的对象。
	  \return Lua中的全局对象是否能精确匹配GMObject。
	*/
	bool getFromGlobal(const char* name, GMObject& obj);

	GMLuaResult protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args = {}, GMVariant* returns = nullptr, GMint32 nRet = 0);

// 针对堆栈的操作，提供给友元
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
	void pushVector(const GMVec2& v);

	bool popVector(GMVec3& v);
	void pushVector(const GMVec3& v);

	bool popVector(GMVec4& v);
	void pushVector(const GMVec4& v);

	void pushMatrix(const GMMat4& v);
	bool popMatrix(GMMat4& v);

	//! 返回Lua的栈顶的变量。
	/*!
	  在调用此方法前，必须确认Lua栈顶是非空的。它将返回Lua栈顶的值但不弹出它，并将其转化为一个GMVariant。需要注意的是，它并不能识别Vec2, Vec3, Vec4和Mat4，因为在Lua中，它们都是表(table)。
	  \return Lua栈顶转化为GMVariant后的值。
	*/
	GMVariant getTop();
	GMVariant get(GMint32 index);
	void push(const GMVariant& var);

public:
	inline GMLuaCoreState* getLuaCoreState() GM_NOEXCEPT
	{
		D(d);
		return d->luaState;
	}

private:
	void loadLibrary();
	GMLuaResult pcall(const char* functionName, const std::initializer_list<GMVariant>& args, GMint32 nRet);
	void setEachMetaMember(const GMObject& obj);
	void setTable(const char* key, const GMObjectMember& value);
	void setMetatable(const GMObject& obj);
};

#undef L
#undef POP_GUARD

#define GM_LUA_FUNC(FuncName) gm::luaapi::GMFunctionReturn FuncName(GMLuaCoreState* L)
#define GM_LUA_PROXY_IMPL(Proxy, FuncName) gm::luaapi::GMFunctionReturn GM_PRIVATE_NAME(Proxy)::FuncName(GMLuaCoreState* L)
#define GM_LUA_PROXY_FUNC(FuncName) GM_META_METHOD gm::luaapi::GMFunctionReturn FuncName(GMLuaCoreState*);

// lua类成员函数相关的宏
#define GM_LUA_PROXY_METATABLE_NAME "__gm_metatable"

#define GM_LUA_PROXY(className) bool __detached = false; className* __handler = nullptr; GMString __name = #className;
#define GM_LUA_PROXY_WITH_TYPE(className, type) bool __detached = false; type* __handler = nullptr; GMString __name = #className;
#define GM_LUA_PROXY_CONSTRUCTOR(proxyName, className) public: proxyName::proxyName(className* handler = nullptr) { D(d); d->__handler = handler; }

#define GM_LUA_PROXY_META GM_META(__detached); GM_META(__handler); GM_META(__name);
#define GM_LUA_PROXY_EXTENDS(clsName) clsName __gm_metatableinstance; gm::GMObject* __gm_metatable = nullptr; // 指定一个表为元表（继承）
#define GM_LUA_PROXY_EXTENDS_META { data()->__gm_metatableinstance.set(get()); data()->__gm_metatable = &(data()->__gm_metatableinstance); GM_META(__gm_metatable); } // registerMeta中加入此宏，像会应用元表
#define GM_LUA_PROXY_OBJECT_NO_DEFAULT_CONSTRUCTOR(className, type)		\
		GM_DECLARE_PRIVATE(className)				\
		GM_ALLOW_COPY_MOVE(className)				\
	public:											\
		void copyData(const GMObject& a)			\
		{											\
			D(d);									\
			const className& i =					\
				dynamic_cast<const className&>(a);	\
			D_OF(d_other, &i);						\
			d->__handler = d_other->__handler;		\
		}											\
													\
		typedef type RealType;						\
		RealType* get()								\
		{											\
			D(d);									\
			return d->__handler;					\
		}											\
													\
		RealType* operator->()						\
		{											\
			return get();							\
		}											\
													\
		operator bool() const						\
		{											\
			D(d);									\
			return !!d->__handler;					\
		}											\
													\
		void detach()								\
		{											\
			D(d);									\
			d->__detached = true;					\
		}											\
													\
		void release()								\
		{											\
			D(d);									\
			if (!d->__detached)						\
			{										\
				GM_delete(d->__handler);			\
				detach();							\
			}										\
		}											\
													\
		void set(RealType* handler)					\
		{											\
			D(d);									\
			d->__handler = handler;					\
		}

#define GM_LUA_PROXY_OBJECT(className, type) \
	GM_LUA_PROXY_CONSTRUCTOR(className, type) \
	GM_LUA_PROXY_OBJECT_NO_DEFAULT_CONSTRUCTOR(className, type)

// Lua indexer，用于模拟GMObject的属性
#define GM_LUA_BEGIN_PROPERTY(proxyClass) \
	{																							\
		static const GMString s_invoker = #proxyClass ".__index";								\
		proxyClass self;																		\
		GMVariant key = GMArgumentHelper::peekArgument(L, 2, s_invoker); /*key*/				\
		HashMap<GMString, std::function<GMReturnValues()>, GMStringHashFunctor> __s_indexMap;	\
		if (__s_indexMap.empty())																\
		{

#define GM_LUA_PROPERTY_PROXY_GETTER(targetProxy, name, memberName) \
		{ __s_indexMap[#memberName] = [&]() {													\
			GMArgumentHelper::popArgument(L, s_invoker); /*key*/								\
			GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/					\
			return GMReturnValues(L, targetProxy(&self->get##name())); }; }

#define GM_LUA_PROPERTY_GETTER(name, memberName) \
		{ __s_indexMap[#memberName] = [&]() {													\
			GMArgumentHelper::popArgument(L, s_invoker); /*key*/								\
			GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/					\
			return GMReturnValues(L, (self->get##name())); }; }

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
				auto value = GMArgumentHelper::popArgument(L, s_invoker); /*value*/																\
				GMArgumentHelper::popArgument(L, s_invoker); /*key*/																			\
				GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/																\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_VEC2)))		\
			{																																	\
				GMVariant value = GMArgumentHelper::popArgumentAsVec2(L, s_invoker); /*value*/													\
				GMArgumentHelper::popArgument(L, s_invoker); /*key*/																			\
				GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/																\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_VEC3)))		\
			{																																	\
				GMVariant value = GMArgumentHelper::popArgumentAsVec3(L, s_invoker); /*value*/													\
				GMArgumentHelper::popArgument(L, s_invoker); /*key*/																			\
				GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/																\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_VEC4)))		\
			{																																	\
				GMVariant value = GMArgumentHelper::popArgumentAsVec4(L, s_invoker); /*value*/													\
				GMArgumentHelper::popArgument(L, s_invoker); /*key*/																			\
				GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/																\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else if (GMString::stringEquals(GM_LUA_PROPERTY_TYPE_NAME(propertyType), GM_LUA_PROPERTY_TYPE_NAME(GM_LUA_PROPERTY_TYPE_MAT4)))		\
			{																																	\
				GMVariant value = GMArgumentHelper::popArgumentAsMat4(L, s_invoker); /*value*/													\
				GMArgumentHelper::popArgument(L, s_invoker); /*key*/																			\
				GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); /*self*/																\
				if (self) self->set##name(value.propertyType());																				\
			}																																	\
			else																																\
			{																																	\
				gm_error(gm_dbg_wrap("wrong lua property type."));																				\
			}																																	\
			return GMReturnValues();																											\
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
	return GMReturnValues();

// Lua模板类，用于模拟一些容器
#define __META(memberName) \
{ \
	GM_STATIC_ASSERT(static_cast<gm::GMMetaMemberType>( gm::GMMetaMemberTypeGetter<decltype(memberName)>::Type ) != gm::GMMetaMemberType::Invalid, "Invalid Meta type"); \
	gm::GMObject::data()->meta[#memberName] = { static_cast<gm::GMMetaMemberType>( gm::GMMetaMemberTypeGetter<decltype(memberName)>::Type ), sizeof(memberName), &memberName }; \
}

#define __META_FUNCTION(memberName) \
	gm::GMObject::data()->meta[#memberName] = { GMMetaMemberType::Function, 0, &memberName };

template <typename T>
class GMLuaVector : public GMObject
{
	typedef Vector<T> ContainerType;
public:
	GMLuaVector() = default;
	GMLuaVector(ContainerType* container)
		: value(container)
	{
	}

	virtual bool registerMeta() override
	{
		__META(__name);
		__META(value);
		__META_FUNCTION(__index);
		__META_FUNCTION(__gc);
		__META_FUNCTION(size);
		return true;
	}

	static gm::luaapi::GMFunctionReturn __gc(GMLuaCoreState* l)
	{
		static const GMString s_invoker = "GMLuaVector.__gc";
		GMLuaVector self;
		GMArgumentHelper::popArgumentAsObject(l, self, s_invoker); //self
		self.release();
		return GMReturnValues();
	}

	static gm::luaapi::GMFunctionReturn __index(GMLuaCoreState* l)
	{
		static const GMString s_invoker = "GMLuaVector.__index";
		GMLuaVector self;
		GMVariant i = GMArgumentHelper::popArgument(l, s_invoker); //i
		GMArgumentHelper::popArgumentAsObject(l, self, s_invoker); //self
		return GMReturnValues(l, (*(self.value))[i.toInt() - 1]);
	}

	static gm::luaapi::GMFunctionReturn size(GMLuaCoreState* l)
	{
		static const GMString s_invoker = "GMLuaVector.size";
		GMLuaVector self;
		GMArgumentHelper::popArgumentAsObject(l, self, s_invoker); //self
		return GMReturnValues(l, self.value->size());
	}

private:
	void release()
	{
		if (value)
			GM_delete(value);
	}

private:
	GMString __name = L"GMLuaVector";
	ContainerType* value = nullptr;
};

#undef __META
#undef __META_FUNCTION

END_NS
#endif