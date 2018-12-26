#ifndef __GM_LUA_ILIGHT_META_H__
#define __GM_LUA_ILIGHT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(ILightProxy)
	{
		GM_LUA_PROXY;
		ILight* light = nullptr;

		GMString __name = "ILight";
		GM_LUA_META_FUNCTION(__gc);
		GM_LUA_META_FUNCTION(setLightAttribute3);
		GM_LUA_META_FUNCTION(setLightAttribute);
	};

	class ILightProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(ILightProxy)
		GM_LUA_META_PROXY_FUNCTIONS(ILight, light)

	public:
		ILightProxy(ILight* light = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif