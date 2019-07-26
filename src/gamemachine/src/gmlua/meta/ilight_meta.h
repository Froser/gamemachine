#ifndef __GM_LUA_ILIGHT_META_H__
#define __GM_LUA_ILIGHT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(ILightProxy);
	class ILightProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(ILight, GMAnyProxy)
		GM_DECLARE_PRIVATE(ILightProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		ILightProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		~ILightProxy();

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif