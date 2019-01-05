#ifndef __GM_LUA_GMMODEL_META_H__
#define __GM_LUA_GMMODEL_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMModelProxy)
	{
		GM_LUA_PROXY(GMModel);
		GM_LUA_PROXY_FUNC(__index);
	};

	class GMModelProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMModelProxy, GMModel)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif