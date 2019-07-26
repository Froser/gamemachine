#include "stdafx.h"
#include "igamehandler_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"

#define NAME "GMWindow"

BEGIN_NS
namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(GameHandlerImpl)
	{
		GMLuaCoreState* L = nullptr;
		GMLuaReference init = 0;
		GMLuaReference start = 0;
		GMLuaReference event = 0;
		GMLuaReference onLoadShaders = 0;
		GMLua* lua = nullptr;
	};

	class GameHandlerImpl : public IGameHandler, public IShaderLoadCallback
	{
		GM_DECLARE_PRIVATE(GameHandlerImpl)
		GM_DECLARE_EMBEDDED_PROPERTY(Init, init)
		GM_DECLARE_EMBEDDED_PROPERTY(Start, start)
		GM_DECLARE_EMBEDDED_PROPERTY(Event, event)
		GM_DECLARE_EMBEDDED_PROPERTY(OnLoadShaders, onLoadShaders)

	public:
		GameHandlerImpl(GMLuaCoreState* L)
		{
			GM_CREATE_DATA();
			D(d);
			GM_ASSERT(L);
			d->L = L;
			d->lua = new GMLua(L);
		}

		~GameHandlerImpl()
		{
			D(d);
			if (d->init)
				d->lua->freeReference(d->init);
			if (d->start)
				d->lua->freeReference(d->start);
			if (d->event)
				d->lua->freeReference(d->event);
			if (d->onLoadShaders)
				d->lua->freeReference(d->onLoadShaders);
		}

	public:
		virtual void init(const IRenderContext* context) override
		{
			D(d);
			IRenderContextProxy renderContext(d->L);
			renderContext.set(context);
			d->lua->protectedCall(d->init, { &renderContext });
		}

		virtual void start() override
		{
			D(d);
			d->lua->protectedCall(d->start);
		}

		virtual void event(GameMachineHandlerEvent evt) override
		{
			D(d);
			d->lua->protectedCall(d->event, { static_cast<GMint32>(evt) });
		}

		virtual void onLoadShaders(const IRenderContext* context) override
		{
			D(d);
			IRenderContextProxy proxy(d->L);
			proxy.set(context);
			d->lua->protectedCall(d->onLoadShaders, { proxy });
		}
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT_UNALIGNED(IGameHandlerProxy)
	{
		IShaderLoadCallback* shaderCallback = nullptr;
		GMLuaReference init = 0;
		GMLuaReference start = 0;
		GMLuaReference event = 0;
		GMLuaReference onLoadShaders = 0;
	};

	IGameHandlerProxy::IGameHandlerProxy(GMLuaCoreState* L)
		: GMAnyProxy(L)
	{
		GM_CREATE_DATA();
		D(d);
		D_BASE(db, Base);
		GameHandlerImpl* impl = new GameHandlerImpl(L);
		db->__handler = impl;
		d->shaderCallback = impl;
	}

	IGameHandlerProxy::~IGameHandlerProxy()
	{

	}

	bool IGameHandlerProxy::registerMeta()
	{
		D(d);
		GM_META(init);
		GM_META(start);
		GM_META(event);
		GM_META(onLoadShaders);
		return Base::registerMeta();
	}

	void IGameHandlerProxy::init()
	{
		D(d);
		D_BASE(db, Base);
		if (db->__handler)
		{
			GameHandlerImpl* impl = static_cast<GameHandlerImpl*>(db->__handler);
			impl->setInit(d->init);
			impl->setEvent(d->event);
			impl->setStart(d->start);
			impl->setOnLoadShaders(d->onLoadShaders);
		}
	}

	IShaderLoadCallback* IGameHandlerProxy::getShaderLoadCallback() GM_NOEXCEPT
	{
		D(d);
		return d->shaderCallback;
	}

}

END_NS
//////////////////////////////////////////////////////////////////////////
