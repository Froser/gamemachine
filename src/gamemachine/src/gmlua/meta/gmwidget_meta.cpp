#include "stdafx.h"
#include "gmwidget_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"
#include "gmbuffer_meta.h"
#include "gmcontrol_meta.h"

BEGIN_NS

namespace luaapi
{

	namespace
	{
		// {{BEGIN META FUNCTION}}
		GM_LUA_NEW_IMPL_ARG(New_GMUIConfiguration, "GMUIConfiguration.new", GMUIConfigurationProxy, GMUIConfiguration, IRenderContextProxy);
		GM_LUA_NEW_IMPL_ARG(New_GMWidgetResourceManager, "GMWidgetResourceManager.new", GMWidgetResourceManagerProxy, GMWidgetResourceManager, IRenderContextProxy);
		// {{END META FUNCTION}}

		GMLuaReg g_meta_gmwidgetresourcemanager[] = {
			// {{BEGIN META DECLARATIONS}}
			{ "new", New_GMWidgetResourceManager },
			// {{END META DECLARATIONS}}
			{ 0, 0 }
		};

		GMLuaReg g_meta_gmuiconfiguration[] = {
			// {{BEGIN META DECLARATIONS}}
			{ "new", New_GMUIConfiguration },
			// {{END META DECLARATIONS}}
			{ 0, 0 }
		};
	}

	GM_LUA_REGISTER_IMPL(GMUIConfiguration_Meta, "GMUIConfiguration", g_meta_gmuiconfiguration)
	GM_LUA_REGISTER_IMPL(GMWidgetResourceManager_Meta, "GMWidgetResourceManager", g_meta_gmwidgetresourcemanager)

	//////////////////////////////////////////////////////////////////////////

	GM_PRIVATE_OBJECT_UNALIGNED(GMUIConfigurationProxy)
	{
		GM_LUA_PROXY_FUNC(initResourceManager);
		GM_LUA_PROXY_FUNC(import);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(GMWidgetResourceManagerProxy)
	{
		GM_LUA_PROXY_FUNC(registerWidget);
		GM_LUA_PROXY_FUNC(createWidget);
	};

	GM_PRIVATE_OBJECT_UNALIGNED(GMWidgetProxy)
	{
		GM_LUA_PROXY_FUNC(setPosition);
		GM_LUA_PROXY_FUNC(setSize);
		GM_LUA_PROXY_FUNC(setTitle);
		GM_LUA_PROXY_FUNC(addControl);
	};


	//////////////////////////////////////////////////////////////////////////

#define NAME "GMUIConfiguration"
	/*
	 * import([self], buffer)
	 */
	GM_LUA_PROXY_IMPL(GMUIConfigurationProxy, import)
	{
		GMLuaArguments args(L, NAME ".import", { GMMetaMemberType::Object, GMMetaMemberType::Object } );
		GMUIConfigurationProxy self(L);
		GMBufferProxy buffer(L);
		args.getArgument(0, &self);
		args.getArgument(1, &buffer);
		if (self)
			self->import(*buffer.get());
		return gm::GMReturnValues();
	}

	/*
	 * registerWidget([self], resourceManager)
	 */
	GM_LUA_PROXY_IMPL(GMUIConfigurationProxy, initResourceManager)
	{
		GMLuaArguments args(L, NAME ".initResourceManager", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		GMUIConfigurationProxy self(L);
		GMWidgetResourceManagerProxy resourceManager(L);
		args.getArgument(0, &self);
		args.getArgument(1, &resourceManager);
		if (self)
			self->initResourceManager(resourceManager.get());
		return gm::GMReturnValues();
	}

	/*
	 * registerWidget([self], widget)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetResourceManagerProxy, registerWidget)
	{
		GMLuaArguments args(L, NAME ".registerWidget", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		GMWidgetResourceManagerProxy self(L);
		GMWidgetProxy widget(L);
		args.getArgument(0, &self);
		args.getArgument(1, &widget);
		if (self)
			self->registerWidget(widget.get());
		return gm::GMReturnValues();
	}

	/*
	 * createWidget([self])
	 */
	GM_LUA_PROXY_IMPL(GMWidgetResourceManagerProxy, createWidget)
	{
		GMLuaArguments args(L, NAME ".createWidget", { GMMetaMemberType::Object });
		GMWidgetResourceManagerProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			GMWidgetProxy widget(L, self->createWidget());
			return gm::GMReturnValues(L, widget);
		}
		return gm::GMReturnValues();
	}

#undef NAME
	//////////////////////////////////////////////////////////////////////////
	// GMWidget
	//////////////////////////////////////////////////////////////////////////
#define NAME "GMWidget"
	/*
	 * setPosition([self], x, y)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, setPosition)
	{
		GMLuaArguments args(L, NAME ".setPosition", { GMMetaMemberType::Object, GMMetaMemberType::Int, GMMetaMemberType::Int });
		GMWidgetProxy self(L);
		args.getArgument(0, &self);
		GMint32 x = args.getArgument(1).toInt();
		GMint32 y = args.getArgument(2).toInt();
		if (self)
			self->setPosition(x, y);
		return gm::GMReturnValues();
	}

	/*
	 * setSize([self], width, height)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, setSize)
	{
		GMLuaArguments args(L, NAME ".setSize", { GMMetaMemberType::Object, GMMetaMemberType::Int, GMMetaMemberType::Int });
		GMWidgetProxy self(L);
		args.getArgument(0, &self);
		GMint32 width = args.getArgument(1).toInt();
		GMint32 height = args.getArgument(2).toInt();
		if (self)
			self->setSize(width, height);
		return gm::GMReturnValues();
	}

	/*
	 * setTitle([self], title)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, setTitle)
	{
		GMLuaArguments args(L, NAME ".setTitle", { GMMetaMemberType::Object, GMMetaMemberType::String });
		GMWidgetProxy self(L);
		args.getArgument(0, &self);
		GMString title = args.getArgument(1).toString();
		if (self)
			self->setTitle(title);
		return gm::GMReturnValues();
	}

	/*
	 * addControl([self], control)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, addControl)
	{
		GMLuaArguments args(L, NAME ".addControl", { GMMetaMemberType::Object, GMMetaMemberType::Object });
		GMWidgetProxy self(L);
		GMControlProxy control(L);
		args.getArgument(0, &self);
		args.getArgument(1, &control);
		if (self)
		{
			self->addControl(control.get());
			control.setAutoRelease(false); // 生命周期由GMWidget管理
		}
		return gm::GMReturnValues();
	}

	//////////////////////////////////////////////////////////////////////////
	bool GMUIConfigurationProxy::registerMeta()
	{
		D(d);
		GM_META_FUNCTION(import);
		GM_META_FUNCTION(initResourceManager);
		return Base::registerMeta();
	}

	GMUIConfigurationProxy::GMUIConfigurationProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	bool GMWidgetResourceManagerProxy::registerMeta()
	{
		D(d);
		GM_META_FUNCTION(registerWidget);
		GM_META_FUNCTION(createWidget);
		return Base::registerMeta();
	}

	GMWidgetResourceManagerProxy::GMWidgetResourceManagerProxy(GMLuaCoreState* l, GMObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	bool GMWidgetProxy::registerMeta()
	{
		D(d);
		GM_META_FUNCTION(setPosition);
		GM_META_FUNCTION(setSize);
		GM_META_FUNCTION(setTitle);
		GM_META_FUNCTION(addControl);
		return Base::registerMeta();
	}

	GMWidgetProxy::GMWidgetProxy(GMLuaCoreState* l, GMObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA()
	}

	GMWidgetProxy::~GMWidgetProxy()
	{

	}
}

END_NS