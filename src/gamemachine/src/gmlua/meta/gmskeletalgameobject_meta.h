#ifndef __GM_LUA_GMSKELETALGAMEOBJECT_META_H__
#define __GM_LUA_GMSKELETALGAMEOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmskeletalgameobject.h>
#include "gmgameobject_meta.h"

BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMSkeletalGameObjectProxy)
	{
		GM_LUA_PROXY;
		GM_LUA_PROXY_METATABLE(GMGameObjectProxy);
		GMSkeletalGameObject* gameObj = nullptr;
		GMString __name = "GMSkeletalGameObject";
		GM_LUA_META_FUNCTION(update);
	};

	class GMSkeletalGameObjectProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMSkeletalGameObjectProxy, GMSkeletalGameObject, gameObj)

	public:
		GMSkeletalGameObjectProxy(GMSkeletalGameObject* gameObj = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	class GMSkeletalGameObject_Meta : public GMLuaFunctionRegister
	{
	public:
		virtual void registerFunctions(GMLua* L) override;

	private:
		static int regCallback(GMLuaCoreState *L);
		static const char* Name;
	};
}

END_NS
#endif