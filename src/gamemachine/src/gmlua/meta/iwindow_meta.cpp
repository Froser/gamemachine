#include "stdafx.h"
#include "iwindow_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

using namespace luaapi;

#define NAME "GMWindow"

namespace
{
	class GMWindowDescProxy : public GMObject
	{
		GM_DECLARE_PRIVATE_FROM_STRUCT(GMWindowDescProxy, GMWindowDesc)

	public:
		const GMWindowDesc& get()
		{
			D(d);
			return *d;
		}

	protected:
		virtual bool registerMeta() override
		{
			GM_META(createNewWindow);
			GM_META(windowName);
			GM_META_WITH_TYPE(dwStyle, GMMetaMemberType::Int);
			GM_META_WITH_TYPE(dwExStyle, GMMetaMemberType::Int);
			GM_META_WITH_TYPE(rc, GMMetaMemberType::Vector4);
			GM_META(samples);
			return true;
		}
	};
}

//////////////////////////////////////////////////////////////////////////
GMWindowProxy::GMWindowProxy(IWindow* window)
{
	D(d);
	d->window = window;
}

bool GMWindowProxy::registerMeta()
{
	D(d);
	GM_META(window);
	GM_META(__name);
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(create);
	GM_META_FUNCTION(centerWindow);
	GM_META_FUNCTION(showWindow);
	GM_META_FUNCTION(setHandler);
	return true;
}

GM_LUA_META_FUNCTION_PROXY_IMPL(GMWindowProxy, __gc, L)
{
	/************************************************************************/
	/* __gc([self])                                                         */
	/************************************************************************/
	static const GMString s_invoker(L"__gc");
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	GMWindowProxy window;
	GMArgumentHelper::popArgumentAsObject(L, window, s_invoker); //self
	if (window)
		window.release();
	return GMReturnValues();
}

GM_LUA_META_FUNCTION_PROXY_IMPL(GMWindowProxy, create, L)
{
	/************************************************************************/
	/* create([self], GMWindowDesc)                                         */
	/************************************************************************/
	static const GMString s_invoker(L"create");
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".create");
	GMWindowProxy window;
	GMWindowDescProxy desc;
	GMArgumentHelper::popArgumentAsObject(L, desc, s_invoker); //GMWindowDesc
	GMArgumentHelper::popArgumentAsObject(L, window, s_invoker); //self
	if (window)
		window->create(desc.get());
	return GMReturnValues();
}

GM_LUA_META_FUNCTION_PROXY_IMPL(GMWindowProxy, centerWindow, L)
{
	/************************************************************************/
	/* centerWindow([self])                                                 */
	/************************************************************************/
	static const GMString s_invoker(L"centerWindow");
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".centerWindow");
	GMWindowProxy window;
	GMArgumentHelper::popArgumentAsObject(L, window, s_invoker); //self
	if (window)
		window->centerWindow();
	return GMReturnValues();
}

GM_LUA_META_FUNCTION_PROXY_IMPL(GMWindowProxy, showWindow, L)
{
	/************************************************************************/
	/* showWindow([self])                                                   */
	/************************************************************************/
	static const GMString s_invoker(L"showWindow");
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".showWindow");
	GMWindowProxy window;
	GMArgumentHelper::popArgumentAsObject(L, window, s_invoker); //self
	if (window)
		window->showWindow();
	return GMReturnValues();
}

GM_LUA_META_FUNCTION_PROXY_IMPL(GMWindowProxy, setHandler, L)
{
	/************************************************************************/
	/* setHandler([self], IGameHandler)                                     */
	/************************************************************************/
	static const GMString s_invoker(L"setHandler");
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setHandler");
	GMWindowProxy window;
	GMArgumentHelper::popArgumentAsObject(L, window, s_invoker); //self
	return GMReturnValues();
}
//////////////////////////////////////////////////////////////////////////
