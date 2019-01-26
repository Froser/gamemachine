#include "stdafx.h"
#include "gmglyphmanager_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmbuffer_meta.h"

using namespace luaapi;

#define NAME "GMGlyphManager"

/*
 * setCN([self], buffer)
 */
GM_LUA_PROXY_IMPL(GMGlyphManagerProxy, addFontByMemory)
{
	static const GMString s_invoker = NAME ".addFontByMemory";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".addFontByMemory");
	GMGlyphManagerProxy self(L);
	GMBufferProxy buffer(L);
	GMArgumentHelper::popArgumentAsObject(L, buffer, s_invoker); //buffer
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		return GMReturnValues(L, self->addFontByMemory(std::move(*buffer.get())));
	return GMReturnValues();
}

/*
 * setEN([self], fontHandle)
 */
GM_LUA_PROXY_IMPL(GMGlyphManagerProxy, setEN)
{
	static const GMString s_invoker = NAME ".setEN";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setEN");
	GMGlyphManagerProxy self(L);
	GMFontHandle fontHandle = static_cast<GMFontHandle>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //fontHandle
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setEN(fontHandle);
	return GMReturnValues();
}

/*
 * setCN([self], fontHandle)
 */
GM_LUA_PROXY_IMPL(GMGlyphManagerProxy, setCN)
{
	static const GMString s_invoker = NAME ".setCN";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setCN");
	GMGlyphManagerProxy self(L);
	GMFontHandle fontHandle = static_cast<GMFontHandle>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //fontHandle
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->setCN(fontHandle);
	return GMReturnValues();
}

bool GMGlyphManagerProxy::registerMeta()
{
	GM_META_FUNCTION(addFontByMemory);
	GM_META_FUNCTION(setEN);
	GM_META_FUNCTION(setCN);
	return Base::registerMeta();
}
