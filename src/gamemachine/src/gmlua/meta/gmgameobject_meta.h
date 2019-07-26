#ifndef __GM_LUA_GAMEOBJECT_META_H__
#define __GM_LUA_GAMEOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmgameobject.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(GMGameObjectProxy);
	class GMGameObjectProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMGameObject, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMGameObjectProxy)
		GM_DECLARE_BASE(GMObjectProxy)

	public:
		GMGameObjectProxy(GMLuaCoreState* l, GMObject* handler = nullptr);
		~GMGameObjectProxy();

	protected:
		virtual bool registerMeta() override;
	};

	GM_LUA_REGISTER(GMGameObject_Meta);
}

END_NS
#endif