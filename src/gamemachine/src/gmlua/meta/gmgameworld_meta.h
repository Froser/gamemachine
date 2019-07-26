#ifndef __GM_LUA_GMGAMEWORLD_META_H__
#define __GM_LUA_GMGAMEWORLD_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmgameworld.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(GMGameWorldProxy);
	class GMGameWorldProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMGameWorld, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMGameWorldProxy)
		GM_DECLARE_BASE(GMObjectProxy)

	public:
		GMGameWorldProxy(GMLuaCoreState* l, GMObject* handler = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	GM_LUA_REGISTER(GMGameWorld_Meta);
}

END_NS
#endif