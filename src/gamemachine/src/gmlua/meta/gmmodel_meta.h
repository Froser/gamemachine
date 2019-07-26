#ifndef __GM_LUA_GMMODEL_META_H__
#define __GM_LUA_GMMODEL_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(GMModelProxy);
	class GMModelProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(GMModel, GMAnyProxy)
		GM_DECLARE_PRIVATE(GMModelProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		GMModelProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		~GMModelProxy();

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif