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
	GM_META(__name);
	GM_META(gameObj);
	GM_META_FUNCTION(update);
	return true;
}

/*
 * update([self], dt)
 */
GM_LUA_PROXY_IMPL(GMSkeletalGameObjectProxy, update, L)
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
	GM_LUA_FUNC(New, L)
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