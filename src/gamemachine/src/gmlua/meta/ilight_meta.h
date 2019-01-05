#ifndef __GM_LUA_ILIGHT_META_H__
#define __GM_LUA_ILIGHT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(ILightProxy)
	{
		GM_LUA_PROXY(ILight);
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(setLightAttribute3);
		GM_LUA_PROXY_FUNC(setLightAttribute);
	};

	class ILightProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(ILightProxy, ILight)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif