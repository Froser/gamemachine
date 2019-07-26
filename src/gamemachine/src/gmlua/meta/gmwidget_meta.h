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

	GM_PRIVATE_CLASS(GMUIConfigurationProxy);
	class GMUIConfigurationProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMUIConfiguration, GMAnyProxy)
		GM_DECLARE_PRIVATE(GMUIConfigurationProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		GMUIConfigurationProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

	public:
		virtual bool registerMeta() override;
	};

	GM_PRIVATE_CLASS(GMWidgetResourceManagerProxy);
	class GMWidgetResourceManagerProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMWidgetResourceManager, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMWidgetResourceManagerProxy)
		GM_DECLARE_BASE(GMObjectProxy)

	public:
		GMWidgetResourceManagerProxy(GMLuaCoreState* l, GMObject* handler = nullptr);

	public:
		virtual bool registerMeta() override;
	};

	GM_PRIVATE_CLASS(GMWidgetProxy);
	class GMWidgetProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMWidget, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMWidgetProxy)
		GM_DECLARE_BASE(GMObjectProxy)

	public:
		GMWidgetProxy(GMLuaCoreState* l, GMObject* handler = nullptr);
		~GMWidgetProxy();

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif