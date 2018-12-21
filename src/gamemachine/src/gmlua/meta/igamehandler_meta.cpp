#include "stdafx.h"
#include "igamehandler_meta.h"
#include <gamemachine.h>
#include <gmlua.h>

using namespace luaapi;

#define NAME "GMWindow"

namespace
{
}

//////////////////////////////////////////////////////////////////////////
IGameHandlerProxy::IGameHandlerProxy(IGameHandler* handler)
{
	D(d);
	d->handler = handler;
}

bool IGameHandlerProxy::registerMeta()
{
	D(d);
	GM_META(handler);
	GM_META(__name);
	GM_META_FUNCTION(__gc);
	return true;
}

GM_LUA_META_FUNCTION_PROXY_IMPL(IGameHandlerProxy, __gc, L)
{
	/************************************************************************/
	/* __gc([self])                                                         */
	/************************************************************************/
	static const GMString s_invoker(L"__gc");
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	IGameHandlerProxy handler;
	GMArgumentHelper::popArgumentAsObject(L, handler, s_invoker); //self
	if (handler)
		handler.release();
	return GMReturnValues();
}

//////////////////////////////////////////////////////////////////////////
