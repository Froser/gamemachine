#ifndef __GM_LUA_GMWIDGET_META_H__
#define __GM_LUA_GMWIDGET_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmwidget.h>
BEGIN_NS

namespace luaapi
{
	GM_LUA_REGISTER(GMWidget_Meta);
	GM_LUA_REGISTER(GMWidgetResourceManager_Meta);

	GM_PRIVATE_OBJECT(GMWidgetResourceManagerProxy)
	{
		GM_LUA_PROXY(GMWidgetResourceManager);
		GM_LUA_PROXY_FUNC(__gc);
	};

	class GMWidgetResourceManagerProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMWidgetResourceManagerProxy, GMWidgetResourceManager)

	public:
		virtual bool registerMeta() override;
	};

	GM_PRIVATE_OBJECT(GMWidgetProxy)
	{
		GM_LUA_PROXY(GMWidget);
		GM_LUA_PROXY_FUNC(__gc);
	};

	class GMWidgetProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMWidgetProxy, GMWidget)

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif