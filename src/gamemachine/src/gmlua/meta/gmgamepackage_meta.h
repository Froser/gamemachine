#ifndef __GM_LUA_GMGAMEPACKAGE_META_H__
#define __GM_LUA_GMGAMEPACKAGE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGamePackageProxy)
	{
		GM_LUA_PROXY(GMGamePackage);
		GM_LUA_PROXY_FUNC(loadPackage)
	};

	class GMGamePackageProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMGamePackageProxy, GMGamePackage)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif