#include "stdafx.h"
#include "gmlua.h"
#include "gmluameta.h"
#include "foundation/utilities/tools.h"
#define L (d->luaState)
#define CHECK(lr) \
	if (lr.state != GMLuaStates::Ok)		\
	{										\
		lr.message = lua_tostring(L, -1);	\
		lua_pop(L, 1);						\
		return lr;							\
	}

BEGIN_NS

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
}

void GMLuaFunctionRegister::setRegisterFunction(GMLua *l, const GMString& modname, GMLuaCFunction openf, bool isGlobal)
{
	GMLuaCoreState* cl = l->getLuaCoreState();
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

GM_PRIVATE_OBJECT_UNALIGNED(GMLua)
{
	GMLuaCoreState* luaState = nullptr;
	GMLuaRuntime luaRuntime;
	bool isWeakLuaStatePtr = false;
	bool libraryLoaded = false;
};

GMLua::GMLua()
{
	GM_CREATE_DATA();
	D(d);
	d->luaState = luaL_newstate();
	d->isWeakLuaStatePtr = false;
}

GMLua::GMLua(lua_State* l)
{
	GM_CREATE_DATA();
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
	GMLuaArguments args(getLuaCoreState());
	args.pushArgument(var);
	lua_setglobal(L, name);
}

GMVariant GMLua::getFromGlobal(const char* name, GMObject* obj)
{
	D(d);
	lua_getglobal(L, name);
	GMLuaArguments args(L);
	GMVariant v = args.getArgument(0, obj);
	lua_pop(L, 1);
	return v;
}

GMLuaResult GMLua::protectedCall(const char* functionName, const std::initializer_list<GMVariant>& args, GMVariant* returns, GMint32 nRet)
{
	D(d);
	GM_ASSERT(functionName);
	lua_getglobal(L, functionName);

	GMLuaResult lr = pcall(args, nRet);
	return pcallreturn(lr, returns, nRet);
}

GMLuaResult GMLua::protectedCall(GMLuaReference funcRef, const std::initializer_list<GMVariant>& args, GMVariant* returns, GMint32 nRet)
{
	D(d);
	lua_rawgeti(L, LUA_REGISTRYINDEX, funcRef);
	GMLuaResult lr = pcall(args, nRet);
	return pcallreturn(lr, returns, nRet);
}

void GMLua::freeReference(GMLuaReference ref)
{
	D(d);
	luaL_unref(L, LUA_REGISTRYINDEX, ref);
}

GMLuaCoreState* GMLua::getLuaCoreState() GM_NOEXCEPT
{
	D(d);
	return d->luaState;
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

GMLuaResult GMLua::pcall(const std::initializer_list<GMVariant>& args, GMint32 nRet)
{
	D(d);
	GMint32 top = lua_gettop(getLuaCoreState()); // top 表示栈在传参之前的索引
	GMLuaArguments a(getLuaCoreState());
	for (const auto& var : args)
	{
		a.pushArgument(var);
	}
	GMint32 currentTop = lua_gettop(getLuaCoreState());

	// 检查压栈数量是否正确
	if (currentTop != top + gm_sizet_to_int(args.size()))
	{
		gm_error(gm_dbg_wrap("Invalid lua stack while calling pcall."));
		GM_ASSERT(false);
		return{ GMLuaStates::RuntimeError, "Invalid lua stack while calling pcall." };
	}

	GMLuaResult lr = { (GMLuaStates)lua_pcall(L, gm_sizet_to_uint(args.size()), nRet, 0) };
	CHECK(lr);

	currentTop = lua_gettop(getLuaCoreState());
	// 检查返回值是否正确
	if (currentTop != top + nRet - 1) // 函数已经被弹出，因此要-1
	{
		gm_error(gm_dbg_wrap("Invalid return value count while calling pcall."));
		GM_ASSERT(false);
		return{ GMLuaStates::RuntimeError, "Invalid return value count while calling pcall." };
	}

	return lr;
}

GMLuaResult GMLua::pcallreturn(GMLuaResult lr, GMVariant* returns, GMint32 nRet)
{
	GMLuaCoreState* l = getLuaCoreState();
	if (lr.state != GMLuaStates::Ok)
	{
		luaL_error(l, "%s", lr.message.toStdString().c_str());
	}
	else
	{
		GMLuaArguments results(getLuaCoreState());
		if (returns)
		{
			for (GMint32 i = 0; i < nRet; i++)
			{
				if (returns[i].isObject())
				{
					results.getArgument(i, returns[i].toObject());
				}
				else
				{
					returns[i] = results.getArgument(i);
				}
			}
		}
	}

	// 清理堆栈
	lua_pop(l, nRet);
	return lr;
}

END_NS