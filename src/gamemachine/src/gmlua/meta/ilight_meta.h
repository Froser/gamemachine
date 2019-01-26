#ifndef __GM_LUA_ILIGHT_META_H__
#define __GM_LUA_ILIGHT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(ILightProxy)
	{
		GM_LUA_PROXY_FUNC(setLightAttribute3);
		GM_LUA_PROXY_FUNC(setLightAttribute);
	};

	class ILightProxy : public GMAnyProxy<ILight>
	{
		GM_LUA_PROXY_OBJ(ILight, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(ILightProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif