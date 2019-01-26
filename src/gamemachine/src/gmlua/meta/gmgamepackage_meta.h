#ifndef __GM_LUA_GMGAMEPACKAGE_META_H__
#define __GM_LUA_GMGAMEPACKAGE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGamePackageProxy)
	{
		GM_LUA_PROXY_FUNC(loadPackage)
		GM_LUA_PROXY_FUNC(readFile)
	};

	class GMGamePackageProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMGamePackage, GMObjectProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMGamePackageProxy, GMObjectProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif