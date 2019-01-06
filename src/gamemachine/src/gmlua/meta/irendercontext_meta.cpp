#include "stdafx.h"
#include "irendercontext_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igraphicengine_meta.h"

using namespace luaapi;

#define NAME "IRenderContext"

bool IRenderContextProxy::registerMeta()
{
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(getWindow);
	GM_META_FUNCTION(getEngine);
	return true;
}

/*
 * getWindow([self])
 */
GM_LUA_PROXY_IMPL(IRenderContextProxy, getWindow)
{
	static const GMString s_invoker = NAME ".getWindow";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getWindow");
	IRenderContextProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		IWindowProxy window(self->getWindow());
		window.detach(); // lua不管理其生命周期
		return GMReturnValues(L, window);
	}
	return GMReturnValues();
}

/*
 * getWindow([self])
 */
GM_LUA_PROXY_IMPL(IRenderContextProxy, getEngine)
{
	static const GMString s_invoker = NAME ".getEngine";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".getEngine");
	IRenderContextProxy self;
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, IGraphicEngineProxy(self->getEngine()));
	return GMReturnValues();
}