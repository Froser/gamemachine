#include "stdafx.h"
#include "gmgamepackage_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmbuffer_meta.h"

using namespace gm::luaapi;

#define NAME "GMGamePackageProxy"

/*
 * loadPackage([self], path)
 */
GM_LUA_PROXY_IMPL(GMGamePackageProxy, loadPackage)
{
	static const GMString s_invoker(L"loadPackage");
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".loadPackage");
	GMGamePackageProxy self;
	GMString path = GMArgumentHelper::popArgumentAsString(L, s_invoker); // path
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->loadPackage(path);
	return GMReturnValues();
}

/*
 * readFile([self], type, filename)
 */
GM_LUA_PROXY_IMPL(GMGamePackageProxy, readFile)
{
	static const GMString s_invoker(L"readFile");
	GM_LUA_CHECK_ARG_COUNT(L, 3, NAME ".readFile");
	GMGamePackageProxy self;
	GMString filename = GMArgumentHelper::popArgumentAsString(L, s_invoker); // filename
	GMPackageIndex type = static_cast<GMPackageIndex>(GMArgumentHelper::popArgument(L, s_invoker).toInt());
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		GMBufferProxy buffer(new GMBuffer());
		self->readFile(type, filename, buffer.get());
		return GMReturnValues(L, buffer);
	}
	return GMReturnValues();
}

bool GMGamePackageProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(loadPackage)
	GM_META_FUNCTION(readFile)
	return true;
}
