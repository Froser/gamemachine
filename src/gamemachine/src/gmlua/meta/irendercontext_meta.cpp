#include "stdafx.h"
#include "irendercontext_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "igraphicengine_meta.h"

#define NAME "IRenderContext"

BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(IRenderContextProxy)
	{
		GM_LUA_PROXY_FUNC(getWindow);
		GM_LUA_PROXY_FUNC(getEngine);
	};

	bool IRenderContextProxy::registerMeta()
	{
		GM_META_FUNCTION(getWindow);
		GM_META_FUNCTION(getEngine);
		return Base::registerMeta();
	}

	void IRenderContextProxy::set(const IRenderContext* o)
	{
		Base::set(const_cast<IRenderContext*>(o));
	}

	void IRenderContextProxy::setLuaCoreState(GMLuaCoreState* l)
	{
		D_BASE(d, Base);
		d->l = l;
	}

	IRenderContextProxy::IRenderContextProxy(const IRenderContextProxy& rhs)
		: GMAnyProxy(rhs.getLuaCoreState(), const_cast<IRenderContext*>(static_cast<const IRenderContext*>(rhs.get())))
	{
		GM_CREATE_DATA();
	}

	IRenderContextProxy::IRenderContextProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
		: Base(l, handler)
	{
		GM_CREATE_DATA();
	}

	IRenderContextProxy::~IRenderContextProxy()
	{
	}

	/*
	 * getWindow([self])
	 */
	GM_LUA_PROXY_IMPL(IRenderContextProxy, getWindow)
	{
		GMLuaArguments args(L, NAME ".getWindow", { GMMetaMemberType::Object });
		IRenderContextProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			IWindowProxy window(L);
			window.set(self->getWindow());
			return GMReturnValues(L, window);
		}
		return GMReturnValues();
	}

	/*
	 * getWindow([self])
	 */
	GM_LUA_PROXY_IMPL(IRenderContextProxy, getEngine)
	{
		GMLuaArguments args(L, NAME ".getEngine", { GMMetaMemberType::Object });
		IRenderContextProxy self(L);
		args.getArgument(0, &self);
		if (self)
		{
			IGraphicEngineProxy engine(L);
			engine.set(self->getEngine());
			return GMReturnValues(L, engine);
		}
		return GMReturnValues();
	}
}
END_NS