#include "stdafx.h"
#include "gmskeletalgameobject_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmasset_meta.h"

#define NAME "GMSkeletalGameObject"

using namespace gm::luaapi;

bool GMSkeletalGameObjectProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_LUA_PROXY_EXTENDS_META;
	GM_META_FUNCTION(update);
	return true;
}

/*
 * update([self], dt)
 */
GM_LUA_PROXY_IMPL(GMSkeletalGameObjectProxy, update)
{
	static const GMString s_invoker = NAME ".update";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".update");
	GMGameObjectProxy self;
	GMfloat dt = GMArgumentHelper::popArgument(L, s_invoker).toFloat(); //duration
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->update(dt);
	return GMReturnValues();
}

namespace
{
	// {{BEGIN META FUNCTION}}
	GM_LUA_DEFAULT_NEW_IMPL(New, NAME ".new", GMSkeletalGameObjectProxy, GMSkeletalGameObject);
	// {{END META FUNCTION}}

	GMLuaReg g_meta[] = {
		// {{BEGIN META DECLARATIONS}}
		{ "new", New },
		// {{END META DECLARATIONS}}
		{ 0, 0 }
	};
}

GM_LUA_REGISTER_IMPL(GMSkeletalGameObject_Meta, NAME, g_meta);