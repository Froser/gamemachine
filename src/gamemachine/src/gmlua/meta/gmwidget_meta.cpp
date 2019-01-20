#include "stdafx.h"
#include "gmwidget_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"
#include "gmbuffer_meta.h"

using namespace luaapi;

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
GM_LUA_REGISTER_IMPL(GMWidgetResourceManager_Meta, "GMWidgetResourceManager", g_meta_gmwidgetresourcemanager);

//////////////////////////////////////////////////////////////////////////
GM_LUA_PROXY_GC_IMPL(GMUIConfigurationProxy, "GMUIConfiguration.__gc");
GM_LUA_PROXY_GC_IMPL(GMWidgetResourceManagerProxy, "GMWidgetResourceManager.__gc");
GM_LUA_PROXY_GC_IMPL(GMWidgetProxy, "GMWidget.__gc");

/*
 * import([self], buffer)
 */
GM_LUA_PROXY_IMPL(GMUIConfigurationProxy, import)
{
	static const GMString s_invoker = "GMUIConfiguration.import";
	GM_LUA_CHECK_ARG_COUNT(L, 2, "GMUIConfiguration.import");
	GMUIConfigurationProxy self;
	GMBufferProxy buffer;
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
	GMUIConfigurationProxy self;
	GMWidgetResourceManagerProxy resourceManager;
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
	GMWidgetResourceManagerProxy self;
	GMWidgetProxy widget;
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
	GMWidgetResourceManagerProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		GMWidgetProxy widget = self->createWidget();
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
	static const GMString s_invoker = "GMWidgetResourceManager.setPosition";
	GM_LUA_CHECK_ARG_COUNT(L, 3, "GMWidgetResourceManager.setPosition");
	GMWidgetProxy self;
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
	static const GMString s_invoker = "GMWidgetResourceManager.setSize";
	GM_LUA_CHECK_ARG_COUNT(L, 3, "GMWidgetResourceManager.setSize");
	GMWidgetProxy self;
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
	static const GMString s_invoker = "GMWidgetResourceManager.setTitle";
	GM_LUA_CHECK_ARG_COUNT(L, 2, "GMWidgetResourceManager.setTitle");
	GMWidgetProxy self;
	GMString title = GMArgumentHelper::popArgumentAsString(L, s_invoker); //title
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setTitle(title);
	return GMReturnValues();
}
//////////////////////////////////////////////////////////////////////////
bool GMUIConfigurationProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(import);
	GM_META_FUNCTION(initResourceManager);
	return true;
}

bool GMWidgetResourceManagerProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(registerWidget);
	GM_META_FUNCTION(createWidget);
	return true;
}

bool GMWidgetProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(setPosition);
	GM_META_FUNCTION(setSize);
	GM_META_FUNCTION(setTitle);
	return true;
}