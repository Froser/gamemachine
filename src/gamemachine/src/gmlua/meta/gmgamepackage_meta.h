#ifndef __GM_LUA_GMGAMEPACKAGE_META_H__
#define __GM_LUA_GMGAMEPACKAGE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(GMGamePackageProxy);
	class GMGamePackageProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMGamePackage, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMGamePackageProxy)
		GM_DECLARE_BASE(GMObjectProxy)

	public:
		GMGamePackageProxy(GMLuaCoreState* l, GMObject* handler = nullptr);
		~GMGamePackageProxy();

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif