#ifndef __DEBUG_H__
#define __DEBUG_H__
#include "common.h"
#include "gmstring.h"
#include <map>
#include <set>

BEGIN_NS

struct GameMachineMessage;
GM_INTERFACE(IDebugOutput)
{
	virtual void info(const GMString& msg) = 0;
	virtual void warning(const GMString& msg) = 0;
	virtual void error(const GMString& msg) = 0;
	virtual void debug(const GMString& msg) = 0;
	virtual bool event(const GameMachineMessage& msg) = 0;
};

GM_PRIVATE_OBJECT(GMDebugger)
{
	IDebugOutput* debugger;
};

class GMDebugger : public GMSingleton<GMDebugger>
{
	DECLARE_PRIVATE(GMDebugger)
	DECLARE_SINGLETON(GMDebugger)

private:
	GMDebugger()
	{
		D(d);
		d->debugger = nullptr;
	}

public:
	static void setDebugOutput(IDebugOutput* output)
	{
		if (instance().data())
			instance().data()->debugger = output;
	}

	static IDebugOutput* getDebugOutput()
	{
		if (instance().data())
			return instance().data()->debugger;
		// 单例已经被析构
		return nullptr;
	}

public:
	void info(const GMWchar* format, ...);
	void info(const char* format, ...);
	void error(const GMWchar* format, ...);
	void error(const char* format, ...);
	void warning(const GMWchar* format, ...);
	void warning(const char* format, ...);
#if _DEBUG
	void debug(const GMWchar* format, ...);
	void debug(const char* format, ...);
#endif
};

// debug macros:
#define gm_info gm::GMDebugger::instance().info
#define gm_error gm::GMDebugger::instance().error
#define gm_warning gm::GMDebugger::instance().warning
#if _DEBUG
#	define gm_debug gm::GMDebugger::instance().debug
#else
#	define gm_debug(i)
#endif

// hooks
#define gm_install_hook(cls, name, funcPtr) { gm::Hooks::install(#cls"_"#name, funcPtr); }
#define gm_hook(cls, name) { gm::Hooks::invoke0(#cls"_"#name)}
#define gm_hook1(cls, name, arg1) { gm::Hooks::invoke1(#cls"_"#name, gm_hook_arg(arg1));}
#define gm_hook2(cls, name, arg1, arg2) { gm::Hooks::invoke2(#cls"_"#name, gm_hook_arg(arg1), gm_hook_arg(arg2));}
#define gm_hook_arg(arg) ((void*)arg)

typedef void(*Hook0)();
typedef void(*Hook1)(void*);
typedef void(*Hook2)(void*, void*);

class Hooks
{
	typedef Set<Hook0> Hook0_t;
	typedef Set<Hook1> Hook1_t;
	typedef Set<Hook2> Hook2_t;

public:
	static void invoke0(const GMString& identifier);
	static void invoke1(const GMString& identifier, void* arg1);
	static void invoke2(const GMString& identifier, void* arg1, void* arg2);
	static void install(const GMString& identifier, Hook0 hook);
	static void install(const GMString& identifier, Hook1 hook);
	static void install(const GMString& identifier, Hook2 hook);

private:
	static Hooks& instance();

private:
	std::map<GMString, Hook0_t> m_hooks0;
	std::map<GMString, Hook1_t> m_hooks1;
	std::map<GMString, Hook2_t> m_hooks2;
};

END_NS
#endif