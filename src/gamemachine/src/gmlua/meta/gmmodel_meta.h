#ifndef __GM_LUA_GMMODEL_META_H__
#define __GM_LUA_GMMODEL_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMModelProxy)
	{
		GM_LUA_PROXY_FUNC(__index);
	};

	class GMModelProxy : public GMAnyProxy<GMModel>
	{
		GM_LUA_PROXY_OBJ(GMModel, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(GMModelProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif