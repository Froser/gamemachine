#include "stdafx.h"
#include "iwindow_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igamehandler_meta.h"
#include "iinput_meta.h"
#include "gmwidget_meta.h"

#define NAME "IWindow"

BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(IWindowProxy)
	{
		GM_LUA_PROXY_FUNC(create);
		GM_LUA_PROXY_FUNC(centerWindow);
		GM_LUA_PROXY_FUNC(showWindow);
		GM_LUA_PROXY_FUNC(setHandler);
		GM_LUA_PROXY_FUNC(getInputManager);
		GM_LUA_PROXY_FUNC(addWidget);
	};

	GM_LUA_PRIVATE_CLASS_FROM(GMWindowDescProxy, GMWindowDesc)
	class GMWindowDescProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMWindowDescProxy);

	public:
		GMWindowDescProxy()
		{
			GM_CREATE_DATA();
		}

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

	//////////////////////////////////////////////////////////////////////////

	bool IWindowProxy::registerMeta()
	{
		D(d);
		GM_META_FUNCTION(create);
		GM_META_FUNCTION(centerWindow);
		GM_META_FUNCTION(showWindow);
		GM_META_FUNCTION(setHandler);
		GM_META_FUNCTION(getInputManager);
		GM_META_FUNCTION(addWidget);
		return Base::registerMeta();
	}

	IWindowProxy::IWindowProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	IWindowProxy::~IWindowProxy()
	{

	}

	/*
	 * create([self], GMWindowDesc)
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, create)
	{
		static const GMString s_invoker = NAME ".create";
		GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".create");
		IWindowProxy self(L);
		GMWindowDescProxy desc;
		GMArgumentHelper::popArgumentAsObject(L, desc, s_invoker); //GMWindowDesc
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->create(desc.get());
		return gm::GMReturnValues();
	}

	/*
	 * centerWindow([self])
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, centerWindow)
	{
		static const GMString s_invoker = NAME ".centerWindow";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".centerWindow");
		IWindowProxy self(L);
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->centerWindow();
		return gm::GMReturnValues();
	}

	/*
	 * showWindow([self])
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, showWindow)
	{
		static const GMString s_invoker = NAME ".showWindow";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".showWindow");
		IWindowProxy self(L);
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->showWindow();
		return gm::GMReturnValues();
	}

	/*
	 * setHandler([self], IGameHandler)
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, setHandler)
	{
		static const GMString s_invoker = NAME ".setHandler";
		GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setHandler");
		IWindowProxy self(L);
		IGameHandlerProxy gameHandler(L);
		GMArgumentHelper::popArgumentAsObject(L, gameHandler, s_invoker); //IGameHandler
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
		{
			gameHandler.setAutoRelease(false);
			gameHandler.init();
			self->setHandler(gameHandler.get());
			self->getGraphicEngine()->setShaderLoadCallback(gameHandler.getShaderLoadCallback());
		}
		return gm::GMReturnValues();
	}

	/*
	 * getInputManager([self])
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, getInputManager)
	{
		static const GMString s_invoker = NAME ".getInputManager";
		GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getInputManager");
		IWindowProxy self(L);
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
		{
			IInputProxy proxy(L);
			proxy.set(self->getInputManager());
			return gm::GMReturnValues(L, proxy);
		}
		return gm::GMReturnValues();
	}

	/*
	 * addWidget([self], widget)
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, addWidget)
	{
		static const GMString s_invoker = NAME ".addWidget";
		GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".addWidget");
		IWindowProxy self(L);
		GMWidgetProxy widget(L);
		GMArgumentHelper::popArgumentAsObject(L, widget, s_invoker); //widget
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->addWidget(widget.get());
		return gm::GMReturnValues();
	}
}
END_NS
//////////////////////////////////////////////////////////////////////////
