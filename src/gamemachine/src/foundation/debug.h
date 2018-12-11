#ifndef __DEBUG_H__
#define __DEBUG_H__
#include <gmcommon.h>
#include "gmstring.h"
#include <gmobject.h>
#include <map>
#include <set>

BEGIN_NS

GM_INTERFACE(IDebugOutput)
{
	typedef void (IDebugOutput::*Method)(const GMString& msg);

	virtual void info(const GMString& msg) = 0;
	virtual void warning(const GMString& msg) = 0;
	virtual void error(const GMString& msg) = 0;
	virtual void debug(const GMString& msg) = 0;
};

GM_PRIVATE_OBJECT_UNALIGNED(GMDebugger)
{
	IDebugOutput* debugger;
};

class GM_EXPORT GMDebugger
{
	GM_DECLARE_PRIVATE_NGO(GMDebugger)

public:
	static GMDebugger& instance();

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

private:
	void info(const GMString& string, const std::initializer_list<GMString>& arguments);
	void warning(const GMString& string, const std::initializer_list<GMString>& arguments);
	void debug(const GMString& string, const std::initializer_list<GMString>& arguments);
	void error(const GMString& string, const std::initializer_list<GMString>& arguments);
	void print(
		GMString string,
		const GMString& prefix,
		IDebugOutput::Method method,
		const std::initializer_list<GMString>& arguments
	);

public:
	template <typename... T>
	void info(const GMString& string, T... args)
	{
		info(string, { args... } );
	}

	template <typename... T>
	void error(const GMString& string, T... args)
	{
		error(string, { args... });
	}

	template <typename... T>
	void warning(const GMString& string, T... args)
	{
		warning(string, { args... });
	}

	template <typename... T>
	void debug(const GMString& string, T... args)
	{
		debug(string, { args... });
	}
};

// debug macros:
#define gm_info gm::GMDebugger::instance().info
#define gm_error gm::GMDebugger::instance().error
#define gm_warning gm::GMDebugger::instance().warning
#define gm_debug gm::GMDebugger::instance().debug

#if GM_WINDOWS
	#if GM_DEBUG
	#	define gm_dbg_wrap(str) __FILE__ ": " __FUNCTION__ ": " str
	#else
	#	define gm_dbg_wrap(str) __FUNCTION__ ": " str
	#endif
#elif GM_UNIX
	#	define gm_dbg_wrap(str) str
#endif

// hooks
typedef GMsize_t CallbackType;
typedef void *HookObject;
struct HookFactory
{
	static Map<CallbackType, HookObject> g_hooks;
};

template <typename... Args>
using HookMap = Map<GMString, Vector<std::function<void(Args...)> > >;

//! 触发一个钩子函数
/*!
  触发对应名称的钩子，调用与其绑定的所有钩子回调函数。
  需要注意的是，拷贝传值、左值引用和右值引用视为不同的回调函数类型，如果绑定的函数类型与触发钩子的参数类型不一致，则会引起错误。
  例如，当调用gm_hook("A hook", 1, 2)时，对应的回调函数类型为void(int, int)。
  如果需要使用带引用的回调函数，必须在gm_hook的模板参数中显示指定出来。如：
  调用gm_hook<int&&, int&&>("A hook", 1, 2)表示触发类型为void(int&&, int&&)的回调函数，参数整数1和2将会作为右值引用传入回调函数。
  \param hookName 钩子名称
  \param args 传入钩子回调函数的参数
  \sa installHook()
*/
template <typename... Args>
inline void hook(const GMString& hookName, Args... args)
{
	typedef std::function<void(Args...)> _FnType;
	typedef Vector<_FnType> _FnListType;

	CallbackType hash_code = typeid(_FnListType).hash_code();
	HookObject hookMap = HookFactory::g_hooks[hash_code];
	if (!hookMap)
		return;

	HookMap<Args...>* hm = static_cast<HookMap<Args...>* >(hookMap);
	_FnListType& callbacks = (*hm)[hookName];
	for (auto& callback : callbacks)
		callback(std::forward<Args>(args)...);
}

//! 安装一个钩子回调。
/*!
  GameMachine提供一套简单但是方便的钩子机制。使用此方法将在全局注册一个钩子回调函数，在被注册的钩子被触发时，此回调函数被调用。
  使用此方法可以为一个钩子绑定任意多个回调函数。
  编译器一般可以推导出模板参数，如假设回调的类型为void(int, int&)，那么以下两种方式等效：
  installHook("hook_name", callback);
  installHook<int, int&>("hook_name", callback);
  \param hookName 钩子的名称。
  \param callback 回调函数。此回调函数的参数可以被编译器自动解析，可以传入返回值为void的任意std::function对象。
  \sa hook()
*/
template <typename... Args>
inline void installHook(const GMString& hookName, std::function<void(Args...)> callback)
{
	typedef Vector<std::function<void(Args...)>> _FnListType;

	CallbackType hash_code = typeid(_FnListType).hash_code();
	HookObject hookMap = HookFactory::g_hooks[hash_code];
	if (hookMap)
	{
		HookMap<Args...>* hm = static_cast<HookMap<Args...>* >(hookMap);
		(*hm)[hookName].push_back(callback);
	}
	else
	{
		static HookMap<Args...> hm;
		hm[hookName].push_back(callback);
		HookFactory::g_hooks[hash_code] = &hm;
	}
}

END_NS
#endif
