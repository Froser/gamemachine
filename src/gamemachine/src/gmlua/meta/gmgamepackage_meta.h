#ifndef __GM_LUA_GMGAMEPACKAGE_META_H__
#define __GM_LUA_GMGAMEPACKAGE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGamePackageProxy)
	{
		GM_LUA_PROXY;
		GMGamePackage* package = nullptr;

		GM_LUA_META_FUNCTION(loadPackage)
	};

	class GMGamePackageProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMGamePackageProxy)
		GM_LUA_META_PROXY_FUNCTIONS(GMGamePackage, package)

	public:
		GMGamePackageProxy(GMGamePackage* package = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif