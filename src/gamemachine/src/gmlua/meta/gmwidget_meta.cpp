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

	/*
	 * import([self], buffer)
	 */
	GM_LUA_PROXY_IMPL(GMUIConfigurationProxy, import)
	{
		static const GMString s_invoker = "GMUIConfiguration.import";
		GM_LUA_CHECK_ARG_COUNT(L, 2, "GMUIConfiguration.import");
		GMUIConfigurationProxy self(L);
		GMBufferProxy buffer(L);
		GMArgumentHelper::popArgumentAsObject(L, buffer, s_invoker); //buffer
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->import(*buffer.get());
		return GMReturnValues();
	}

	/*
	 * registerWidget([self], resourceManager)
	 */
	GM_LUA_PROXY_IMPL(GMUIConfigurationProxy, initResourceManager)
	{
		static const GMString s_invoker = "GMUIConfiguration.initResourceManager";
		GM_LUA_CHECK_ARG_COUNT(L, 2, "GMUIConfiguration.initResourceManager");
		GMUIConfigurationProxy self(L);
		GMWidgetResourceManagerProxy resourceManager(L);
		GMArgumentHelper::popArgumentAsObject(L, resourceManager, s_invoker); //resourceManager
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->initResourceManager(resourceManager.get());
		return GMReturnValues();
	}

	/*
	 * registerWidget([self], widget)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetResourceManagerProxy, registerWidget)
	{
		static const GMString s_invoker = "GMWidgetResourceManager.registerWidget";
		GM_LUA_CHECK_ARG_COUNT(L, 2, "GMWidgetResourceManager.registerWidget");
		GMWidgetResourceManagerProxy self(L);
		GMWidgetProxy widget(L);
		GMArgumentHelper::popArgumentAsObject(L, widget, s_invoker); //widget
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->registerWidget(widget.get());
		return GMReturnValues();
	}

	/*
	 * createWidget([self])
	 */
	GM_LUA_PROXY_IMPL(GMWidgetResourceManagerProxy, createWidget)
	{
		static const GMString s_invoker = "GMWidgetResourceManager.createWidget";
		GM_LUA_CHECK_ARG_COUNT(L, 1, "GMWidgetResourceManager.createWidget");
		GMWidgetResourceManagerProxy self(L);
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
		{
			GMWidgetProxy widget(L, self->createWidget());
			return GMReturnValues(L, widget);
		}
		return GMReturnValues();
	}

	//////////////////////////////////////////////////////////////////////////
	// GMWidget
	//////////////////////////////////////////////////////////////////////////
	/*
	 * setPosition([self], x, y)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, setPosition)
	{
		static const GMString s_invoker = "GMWidget.setPosition";
		GM_LUA_CHECK_ARG_COUNT(L, 3, "GMWidget.setPosition");
		GMWidgetProxy self(L);
		GMint32 y = GMArgumentHelper::popArgument(L, s_invoker).toInt(); //y
		GMint32 x = GMArgumentHelper::popArgument(L, s_invoker).toInt(); //x
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->setPosition(x, y);
		return GMReturnValues();
	}

	/*
	 * setSize([self], width, height)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, setSize)
	{
		static const GMString s_invoker = "GMWidget.setSize";
		GM_LUA_CHECK_ARG_COUNT(L, 3, "GMWidget.setSize");
		GMWidgetProxy self(L);
		GMint32 height = GMArgumentHelper::popArgument(L, s_invoker).toInt(); //height
		GMint32 width = GMArgumentHelper::popArgument(L, s_invoker).toInt(); //width
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->setSize(width, height);
		return GMReturnValues();
	}

	/*
	 * setTitle([self], title)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, setTitle)
	{
		static const GMString s_invoker = "GMWidget.setTitle";
		GM_LUA_CHECK_ARG_COUNT(L, 2, "GMWidget.setTitle");
		GMWidgetProxy self(L);
		GMString title = GMArgumentHelper::popArgumentAsString(L, s_invoker); //title
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->setTitle(title);
		return GMReturnValues();
	}

	/*
	 * addControl([self], control)
	 */
	GM_LUA_PROXY_IMPL(GMWidgetProxy, addControl)
	{
		static const GMString s_invoker = "GMWidget.addControl";
		GM_LUA_CHECK_ARG_COUNT(L, 2, "GMWidget.addControl");
		GMWidgetProxy self(L);
		GMControlProxy control(L);
		GMArgumentHelper::popArgumentAsObject(L, control, s_invoker); //control
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
		{
			self->addControl(control.get());
			control.setAutoRelease(false); // 生命周期由GMWidget管理
		}
		return GMReturnValues();
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