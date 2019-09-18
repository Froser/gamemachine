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
		GMLuaArguments args(L, NAME ".create", { GMMetaMemberType::Object, GMMetaMemberType::Object } );
		IWindowProxy self(L);
		GMWindowDescProxy desc;
		args.getArgument(0, &self);
		args.getArgument(1, &desc);
		if (self)
			self->create(desc.get());
		return gm::GMReturnValues();
	}

	/*
	 * centerWindow([self])
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, centerWindow)
	{
		GMLuaArguments args(L, NAME ".centerWindow", { GMMetaMemberType::Object });
		IWindowProxy self(L);
		args.getArgument(0, &self);
		if (self)
			self->centerWindow();
		return gm::GMReturnValues();
	}

	/*
	 * showWindow([self])
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, showWindow)
	{
		GMLuaArguments args(L, NAME ".showWindow", { GMMetaMemberType::Object });
		IWindowProxy self(L);
		args.getArgument(0, &self);
		if (self)
			self->showWindow();
		return gm::GMReturnValues();
	}

	/*
	 * setHandler([self], IGameHandler)
	 */
	GM_LUA_PROXY_IMPL(IWindowProxy, setHandler)
	{
		GMLuaArguments args(L, NAME ".setHandler", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		IWindowProxy self(L);
		IGameHandlerProxy gameHandler(L);
		args.getArgument(0, &self);
		args.getArgument(1, &gameHandler);
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
		GMLuaArguments args(L, NAME ".getInputManager", { GMMetaMemberType::Object });
		IWindowProxy self(L);
		args.getArgument(0, &self);
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
		GMLuaArguments args(L, NAME ".addWidget", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		IWindowProxy self(L);
		GMWidgetProxy widget(L);
		args.getArgument(0, &self);
		args.getArgument(1, &widget);
		if (self)
			self->addWidget(widget.get());
		return gm::GMReturnValues();
	}
}
END_NS
//////////////////////////////////////////////////////////////////////////
