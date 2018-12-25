#include "stdafx.h"
#include "gmgamepackage_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

using namespace gm::luaapi;

#define NAME "GMGamePackageProxy"

/*
* loadPackage([self], path)
*/
GM_LUA_META_FUNCTION_PROXY_IMPL(GMGamePackageProxy, loadPackage, L)
{
	static const GMString s_invoker(L"loadPackage");
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".loadPackage");
	GMGamePackageProxy self;
	GMString path = GMArgumentHelper::popArgumentAsString(L, s_invoker); // path
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->loadPackage(path);
	return 0;
}

GMGamePackageProxy::GMGamePackageProxy(GMGamePackage* package)
{
	D(d);
	d->package = package;
}

bool GMGamePackageProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(package)
	GM_META_FUNCTION(loadPackage)
	return true;
}
