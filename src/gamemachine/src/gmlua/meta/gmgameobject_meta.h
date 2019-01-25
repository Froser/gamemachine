#ifndef __GM_LUA_GAMEOBJECT_META_H__
#define __GM_LUA_GAMEOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmgameobject.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMGameObjectProxy)
	{
		GM_LUA_PROXY_FUNC(setAsset);
		GM_LUA_PROXY_FUNC(setTranslation);
		GM_LUA_PROXY_FUNC(setRotation);
		GM_LUA_PROXY_FUNC(setScaling);
	};

	class GMGameObjectProxy : public GMObjectProxy
	{
		GM_LUA_PROXY_OBJ(GMGameObjectProxy, GMGameObject, GMObjectProxy)
		GM_DECLARE_PRIVATE(GMGameObjectProxy)

	protected:
		virtual bool registerMeta() override;
	};

	GM_LUA_REGISTER(GMGameObject_Meta);
}

END_NS
#endif