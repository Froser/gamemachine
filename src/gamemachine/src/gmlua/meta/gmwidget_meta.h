#ifndef __GM_LUA_GMWIDGET_META_H__
#define __GM_LUA_GMWIDGET_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmwidget.h>
#include <gmuiconfiguration.h>
BEGIN_NS

namespace luaapi
{
	GM_LUA_REGISTER(GMWidgetResourceManager_Meta);
	GM_LUA_REGISTER(GMUIConfiguration_Meta);

	GM_PRIVATE_OBJECT(GMUIConfigurationProxy)
	{
		GM_LUA_PROXY(GMUIConfiguration);
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(initResourceManager);
		GM_LUA_PROXY_FUNC(import);
	};

	class GMUIConfigurationProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMUIConfigurationProxy, GMUIConfiguration)

	public:
		virtual bool registerMeta() override;
	};

	GM_PRIVATE_OBJECT(GMWidgetResourceManagerProxy)
	{
		GM_LUA_PROXY(GMWidgetResourceManager);
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(registerWidget);
		GM_LUA_PROXY_FUNC(createWidget);
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
		GM_LUA_PROXY_FUNC(setPosition);
		GM_LUA_PROXY_FUNC(setSize);
		GM_LUA_PROXY_FUNC(setTitle);
		GM_LUA_PROXY_FUNC(addControl);
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