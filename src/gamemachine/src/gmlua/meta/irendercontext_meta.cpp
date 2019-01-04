#include "stdafx.h"
#include "irendercontext_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igraphicengine_meta.h"

using namespace luaapi;

#define NAME "IRenderContext"

IRenderContextProxy::IRenderContextProxy(const IRenderContext* context)
{
	D(d);
	d->context = context;
}

bool IRenderContextProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META(context);
	GM_META_FUNCTION(getWindow);
	GM_META_FUNCTION(getEngine);
	return true;
}

/*
 * getWindow([self])
 */
GM_LUA_PROXY_IMPL(IRenderContextProxy, getWindow, L)
{
	static const GMString s_invoker = NAME ".getWindow";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getWindow");
	IRenderContextProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	IWindowProxy window(self->getWindow());
	return GMReturnValues(L, window);
}

/*
 * getWindow([self])
 */
GM_LUA_PROXY_IMPL(IRenderContextProxy, getEngine, L)
{
	static const GMString s_invoker = NAME ".getEngine";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getEngine");
	IRenderContextProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	IGraphicEngineProxy engine(self->getEngine());
	return GMReturnValues(L, engine);
}