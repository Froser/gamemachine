#include "stdafx.h"
#include "iwindow_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igamehandler_meta.h"

using namespace luaapi;

#define NAME "IWindow"

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
IWindowProxy::IWindowProxy(IWindow* window)
{
	D(d);
	d->window = window;
}

bool IWindowProxy::registerMeta()
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

/*
 * __gc([self], GMWindowDesc)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(IWindowProxy, __gc, L)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	IWindowProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self.release();
	return GMReturnValues();
}

/*
 * create([self], GMWindowDesc)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(IWindowProxy, create, L)
{
	static const GMString s_invoker = NAME ".create";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".create");
	IWindowProxy self;
	GMWindowDescProxy desc;
	GMArgumentHelper::popArgumentAsObject(L, desc, s_invoker); //GMWindowDesc
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->create(desc.get());
	return GMReturnValues();
}

/*
 * centerWindow([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(IWindowProxy, centerWindow, L)
{
	static const GMString s_invoker = NAME ".centerWindow";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".centerWindow");
	IWindowProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->centerWindow();
	return GMReturnValues();
}

/*
 * showWindow([self])
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(IWindowProxy, showWindow, L)
{
	static const GMString s_invoker = NAME ".showWindow";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".showWindow");
	IWindowProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->showWindow();
	return GMReturnValues();
}

/*
 * setHandler([self], IGameHandler)
 */
GM_LUA_META_FUNCTION_PROXY_IMPL(IWindowProxy, setHandler, L)
{
	static const GMString s_invoker = NAME ".setHandler";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setHandler");
	IWindowProxy self;
	IGameHandlerProxy gameHandler(L);
	GMArgumentHelper::popArgumentAsObject(L, gameHandler, s_invoker); //IGameHandler
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		gameHandler.init();
		self->setHandler(gameHandler.get());
		self->getGraphicEngine()->setShaderLoadCallback(gameHandler.getShaderLoadCallback());
	}
	return GMReturnValues();
}
//////////////////////////////////////////////////////////////////////////
