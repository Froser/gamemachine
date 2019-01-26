#ifndef __GM_LUA_GMWIDGET_META_H__
#define __GM_LUA_GMWIDGET_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmwidget.h>
#include <gmuiconfiguration.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_LUA_REGISTER(GMWidgetResourceManager_Meta);
	GM_LUA_REGISTER(GMUIConfiguration_Meta);

	GM_PRIVATE_OBJECT(GMUIConfigurationProxy)
	{
		GM_LUA_PROXY_FUNC(initResourceManager);
		GM_LUA_PROXY_FUNC(import);
	};

	class GMUIConfigurationProxy : public GMAnyProxy<GMUIConfiguration>
	{
		GM_LUA_PROXY_OBJ(GMUIConfiguration, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMUIConfigurationProxy, GMAnyProxy)

	public:
		virtual bool registerMeta() override;
	};

	GM_PRIVATE_OBJECT(GMWidgetResourceManagerProxy)
	{
		GM_LUA_PROXY_FUNC(registerWidget);
		GM_LUA_PROXY_FUNC(createWidget);
	};

	class GMWidgetResourceManagerProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMWidgetResourceManager, GMObjectProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMWidgetResourceManagerProxy, GMObjectProxy)

	public:
		virtual bool registerMeta() override;
	};

	GM_PRIVATE_OBJECT(GMWidgetProxy)
	{
		GM_LUA_PROXY_FUNC(setPosition);
		GM_LUA_PROXY_FUNC(setSize);
		GM_LUA_PROXY_FUNC(setTitle);
		GM_LUA_PROXY_FUNC(addControl);
	};

	class GMWidgetProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMWidget, GMObjectProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMWidgetProxy, GMObjectProxy)

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif