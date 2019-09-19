#include "stdafx.h"
#include "gmgamepackage_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "gmbuffer_meta.h"

#define NAME "GMGamePackageProxy"

BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(GMGamePackageProxy)
	{
		GM_LUA_PROXY_FUNC(loadPackage)
		GM_LUA_PROXY_FUNC(readFile)
	};

	/*
	 * loadPackage([self], path)
	 */
	GM_LUA_PROXY_IMPL(GMGamePackageProxy, loadPackage)
	{
		static const GMString s_invoker(L"loadPackage");
		GMLuaArguments args(L, NAME ".loadPackage", { GMMetaMemberType::Object, GMMetaMemberType::String } );
		GMGamePackageProxy self(L);
		args.getArgument(0, &self);
		GMString path = args.getArgument(1).toString();
		if (self)
			self->loadPackage(path);
		return GMReturnValues();
	}

	/*
	 * readFile([self], type, filename)
	 */
	GM_LUA_PROXY_IMPL(GMGamePackageProxy, readFile)
	{
		static const GMString s_invoker(NAME ".readFile");
		GMLuaArguments args(L, NAME ".readFile", { GMMetaMemberType::Object, GMMetaMemberType::Int, GMMetaMemberType::String });
		GMGamePackageProxy self(L);
		args.getArgument(0, &self);
		GMPackageIndex type = static_cast<GMPackageIndex>(args.getArgument(1).toInt());
		GMString filename = args.getArgument(2).toString();
		if (self)
		{
			GMBufferProxy buffer(L, new GMBuffer());
			self->readFile(type, filename, buffer.get());
			return GMReturnValues(L, buffer);
		}
		return GMReturnValues();
	}

	bool GMGamePackageProxy::registerMeta()
	{
		GM_META_FUNCTION(loadPackage)
		GM_META_FUNCTION(readFile)
		return Base::registerMeta();
	}

	GMGamePackageProxy::GMGamePackageProxy(GMLuaCoreState* l, GMObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	GMGamePackageProxy::~GMGamePackageProxy()
	{

	}
}

END_NS