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
		GM_LUA_PROXY(GMSkeletalGameObject);
		GM_LUA_PROXY_EXTENDS(GMGameObjectProxy);
		GM_LUA_PROXY_FUNC(update);
	};

	class GMSkeletalGameObjectProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMSkeletalGameObjectProxy, GMSkeletalGameObject)

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