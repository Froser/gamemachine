#include "stdafx.h"
#include "iwindow_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igamehandler_meta.h"
#include "iinput_meta.h"
#include "gmwidget_meta.h"

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
bool IWindowProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(create);
	GM_META_FUNCTION(centerWindow);
	GM_META_FUNCTION(showWindow);
	GM_META_FUNCTION(setHandler);
	GM_META_FUNCTION(getInputManager);
	GM_META_FUNCTION(addWidget);
	return true;
}

GM_LUA_PROXY_GC_IMPL(IWindowProxy, "IWindow.__gc");

/*
 * create([self], GMWindowDesc)
 */
GM_LUA_PROXY_IMPL(IWindowProxy, create)
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
GM_LUA_PROXY_IMPL(IWindowProxy, centerWindow)
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
GM_LUA_PROXY_IMPL(IWindowProxy, showWindow)
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
GM_LUA_PROXY_IMPL(IWindowProxy, setHandler)
{
	static const GMString s_invoker = NAME ".setHandler";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setHandler");
	IWindowProxy self;
	IGameHandlerProxy gameHandler(L);
	GMArgumentHelper::beginArgumentReference(L, gameHandler, s_invoker); //IGameHandler
	gameHandler.detach();
	GMArgumentHelper::endArgumentReference(L, gameHandler);
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		gameHandler.init();
		self->setHandler(gameHandler.get());
		self->getGraphicEngine()->setShaderLoadCallback(gameHandler.getShaderLoadCallback());
	}
	return GMReturnValues();
}

/*
 * getInputManager([self])
 */
GM_LUA_PROXY_IMPL(IWindowProxy, getInputManager)
{
	static const GMString s_invoker = NAME ".getInputManager";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getInputManager");
	IWindowProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, IInputProxy(self->getInputManager()));
	return GMReturnValues();
}

/*
 * addWidget([self], widget)
 */
GM_LUA_PROXY_IMPL(IWindowProxy, addWidget)
{
	static const GMString s_invoker = NAME ".addWidget";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".addWidget");
	IWindowProxy self;
	GMWidgetProxy widget;
	GMArgumentHelper::popArgumentAsObject(L, widget, s_invoker); //widget
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->addWidget(widget.get());
	return GMReturnValues();
}
//////////////////////////////////////////////////////////////////////////
