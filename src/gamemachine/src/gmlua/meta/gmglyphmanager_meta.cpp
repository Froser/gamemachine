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
		GMLuaArguments args(L, NAME ".addFontByMemory", { GMMetaMemberType::Object, GMMetaMemberType::Object } );
		GMGlyphManagerProxy self(L);
		GMBufferProxy buffer(L);
		args.getArgument(0, &self);
		args.getArgument(1, &buffer);
		if (self)
			return gm::GMReturnValues(L, self->addFontByMemory(std::move(*buffer.get())));
		return gm::GMReturnValues();
	}

	/*
	 * setDefaultFontEN([self], fontHandle)
	 */
	GM_LUA_PROXY_IMPL(GMGlyphManagerProxy, setDefaultFontEN)
	{
		GMLuaArguments args(L, NAME ".setDefaultFontEN", { GMMetaMemberType::Object, GMMetaMemberType::Int });
		GMGlyphManagerProxy self(L);
		args.getArgument(0, &self);
		GMFontHandle fontHandle = static_cast<GMFontHandle>(args.getArgument(1).toInt()); //fontHandle
		if (self)
			self->setDefaultFontEN(fontHandle);
		return gm::GMReturnValues();
	}

	/*
	 * setDefaultFontCN([self], fontHandle)
	 */
	GM_LUA_PROXY_IMPL(GMGlyphManagerProxy, setDefaultFontCN)
	{
		GMLuaArguments args(L, NAME ".setDefaultFontCN", { GMMetaMemberType::Object, GMMetaMemberType::Int });
		GMGlyphManagerProxy self(L);
		GMFontHandle fontHandle = static_cast<GMFontHandle>(args.getArgument(1).toInt()); //fontHandle
		if (self)
			self->setDefaultFontCN(fontHandle);
		return gm::GMReturnValues();
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