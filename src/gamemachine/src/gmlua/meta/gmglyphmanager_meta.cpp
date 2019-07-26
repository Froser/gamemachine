#include "stdafx.h"
#include "gmglyphmanager_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmbuffer_meta.h"

BEGIN_NS
#define NAME "GMGlyphManager"

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(GMGlyphManagerProxy)
	{
		GM_LUA_PROXY_FUNC(addFontByMemory);
		GM_LUA_PROXY_FUNC(setDefaultFontEN);
		GM_LUA_PROXY_FUNC(setDefaultFontCN);
	};

	/*
	 * addFontByMemory([self], buffer)
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
	 * setDefaultFontEN([self], fontHandle)
	 */
	GM_LUA_PROXY_IMPL(GMGlyphManagerProxy, setDefaultFontEN)
	{
		static const GMString s_invoker = NAME ".setDefaultFontEN";
		GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setDefaultFontEN");
		GMGlyphManagerProxy self(L);
		GMFontHandle fontHandle = static_cast<GMFontHandle>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //fontHandle
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->setDefaultFontEN(fontHandle);
		return GMReturnValues();
	}

	/*
	 * setDefaultFontCN([self], fontHandle)
	 */
	GM_LUA_PROXY_IMPL(GMGlyphManagerProxy, setDefaultFontCN)
	{
		static const GMString s_invoker = NAME ".setDefaultFontCN";
		GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".setDefaultFontCN");
		GMGlyphManagerProxy self(L);
		GMFontHandle fontHandle = static_cast<GMFontHandle>(GMArgumentHelper::popArgument(L, s_invoker).toInt()); //fontHandle
		GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
		if (self)
			self->setDefaultFontCN(fontHandle);
		return GMReturnValues();
	}

	bool GMGlyphManagerProxy::registerMeta()
	{
		GM_META_FUNCTION(addFontByMemory);
		GM_META_FUNCTION(setDefaultFontEN);
		GM_META_FUNCTION(setDefaultFontCN);
		return Base::registerMeta();
	}

	GMGlyphManagerProxy::GMGlyphManagerProxy(GMLuaCoreState* l, GMObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	GMGlyphManagerProxy::~GMGlyphManagerProxy()
	{

	}

}
END_NS