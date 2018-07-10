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
class GMLua;

struct GMLuaFunctionRegister
{
	virtual void registerFunctions(GMLua*) = 0;

protected:
	void setRegisterFunction(GMLua *l, const GMString& modname, GMLuaCFunction openf, bool isGlobal);
	
protected:
	static void newLibrary(GMLuaCoreState *l, const GMLuaReg* functions);
};

// 检查栈是否平衡
#if _DEBUG
#	define GM_CHECK_LUA_STACK_BALANCE(offset) gm::GMLuaStackBalanceCheck __balanceGuard(L, offset);
#	define GM_CHECK_LUA_STACK_BALANCE_(l, offset) gm::GMLuaStackBalanceCheck __balanceGuard(l, offset);

class GMLuaStackBalanceCheck
{
public:
	GMLuaStackBalanceCheck(GMLuaCoreState* l, GMint offset)
		: m_l(l)
		, m_offset(offset)
	{
		m_stack = lua_gettop(m_l);
	}

	~GMLuaStackBalanceCheck()
	{
#if _DEBUG
		auto currentTop = lua_gettop(m_l);
		GM_ASSERT(currentTop == m_stack + m_offset);
#endif
	}

private:
	GMint m_stack;
	GMint m_offset;
	GMLuaCoreState* m_l;
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
	lua_State* luaState = nullptr;
	bool isWeakLuaStatePtr = false;
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

	//! 调用一个表达式。
	/*!
	在当前上下文中，调用一个表达式语句。
	\param expr 所调用的表达式。
	\return 是否调用成功。
	*/
	GMLuaResult runString(const GMString& string);

	void setToGlobal(const char* name, const GMVariant& var);
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

	GMLuaResult protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args, GMVariant* returns = nullptr, GMint nRet = 0);

	//! 将一个对象的成员压入Lua的虚拟堆栈。
	/*!
	  在Lua C函数被调用时，使用此方法，将返回一个Lua table。
	  \param obj 待传入的对象。此对象必须要注册元对象。
	  \sa GMObject::registerMeta()
	*/
	void pushTable(const GMObject& obj);

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
	bool popTable(GMObject& obj, GMint index);

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

public:
	inline GMLuaCoreState* getLuaCoreState() GM_NOEXCEPT
	{
		D(d);
		return d->luaState;
	}

private:
	void loadLibrary();
	void registerLibraries();
	GMLuaResult pcall(const char* functionName, const std::initializer_list<GMVariant>& args, GMint nRet);
	void push(const GMVariant& var);
	void push(const char* name, const GMObjectMember& member);
};

#undef L
#undef POP_GUARD
END_NS
#endif