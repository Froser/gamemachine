#include "stdafx.h"
#include "gmwidget_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"

using namespace luaapi;

namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_NEW_IMPL_ARG(New_GMWidgetResourceManager, "GMWidgetResourceManager.new", GMWidgetResourceManagerProxy, GMWidgetResourceManager, IRenderContextProxy);
	GM_LUA_NEW_IMPL_ARG(New_GMWidget, "GMWidget.new", GMWidgetProxy, GMWidget, GMWidgetResourceManagerProxy);
	// {{END META FUNCTION}}

	GMLuaReg g_meta_gmwidget[] = {
		// {{BEGIN META DECLARATIONS}}
		{ "new", New_GMWidget },
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};

	GMLuaReg g_meta_gmwidgetresourcemanager[] = {
		// {{BEGIN META DECLARATIONS}}
		{ "new", New_GMWidgetResourceManager },
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

GM_LUA_REGISTER_IMPL(GMWidget_Meta, "GMWidget", g_meta_gmwidget);
GM_LUA_REGISTER_IMPL(GMWidgetResourceManager_Meta, "GMWidgetResourceManager", g_meta_gmwidgetresourcemanager);

//////////////////////////////////////////////////////////////////////////
GM_LUA_PROXY_GC_IMPL(GMWidgetResourceManagerProxy, "IWindow.__gc");
GM_LUA_PROXY_GC_IMPL(GMWidgetProxy, "IWindow.__gc");

bool GMWidgetResourceManagerProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	return true;
}

bool GMWidgetProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	return true;
}
