#include "stdafx.h"
#include "gmskeletalgameobject_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmasset_meta.h"

#define NAME "GMSkeletalGameObject"

using namespace gm::luaapi;

GMSkeletalGameObjectProxy::GMSkeletalGameObjectProxy(GMSkeletalGameObject* gameObj /*= nullptr*/)
{
	D(d);
	d->gameObj = gameObj;
}

bool GMSkeletalGameObjectProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_LUA_PROXY_METATABLE_META;
	GM_META(gameObj);
	return true;
}

namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_META_FUNCTION_IMPL(New, L)
	{
		static const GMString s_invoker = NAME ".new";
		GM_LUA_CHECK_ARG_COUNT(L, 0, NAME ".new");
		GMSkeletalGameObjectProxy proxy(new GMSkeletalGameObject());
		return GMReturnValues(L, GMVariant(proxy));
	}
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		{ "new", New },
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

const char* GMSkeletalGameObject_Meta::Name = NAME;

void GMSkeletalGameObject_Meta::registerFunctions(GMLua* L)
{
	setRegisterFunction(L, Name, regCallback, true);
}

int GMSkeletalGameObject_Meta::regCallback(GMLuaCoreState *L)
{
	newLibrary(L, g_meta);
	return 1;
}