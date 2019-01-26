#include "stdafx.h"
#include "igamehandler_meta.h"
#include <gamemachine.h>
#include <gmlua.h>
#include "irendercontext_meta.h"

using namespace luaapi;

#define NAME "GMWindow"

namespace
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
		GM_DECLARE_PRIVATE_NGO(GameHandlerImpl)
		GM_DECLARE_PROPERTY(Init, init)
		GM_DECLARE_PROPERTY(Start, start)
		GM_DECLARE_PROPERTY(Event, event)
		GM_DECLARE_PROPERTY(OnLoadShaders, onLoadShaders)

	public:
		GameHandlerImpl(GMLuaCoreState* L)
		{
			D(d);
			GM_ASSERT(L);
			d->L = L;
			d->lua = new GMLua(L);
		}

		~GameHandlerImpl()
		{
			D(d);
			if (d->init)
				luaL_unref(d->L, LUA_REGISTRYINDEX, d->init);
			if (d->start)
				luaL_unref(d->L, LUA_REGISTRYINDEX, d->start);
			if (d->event)
				luaL_unref(d->L, LUA_REGISTRYINDEX, d->event);
			if (d->onLoadShaders)
				luaL_unref(d->L, LUA_REGISTRYINDEX, d->onLoadShaders);
		}

	public:
		virtual void init(const IRenderContext* context) override
		{
			D(d);
			IRenderContextProxy renderContext(d->L);
			renderContext.set(context);
			lua_rawgeti(d->L, LUA_REGISTRYINDEX, d->init);
			d->lua->protectedCall(nullptr, { &renderContext });
		}

		virtual void start() override
		{
			D(d);
			lua_rawgeti(d->L, LUA_REGISTRYINDEX, d->start);
			d->lua->protectedCall(nullptr);
		}

		virtual void event(GameMachineHandlerEvent evt) override
		{
			D(d);
			lua_rawgeti(d->L, LUA_REGISTRYINDEX, d->event);
			d->lua->protectedCall(nullptr, { static_cast<GMint32>(evt) });
		}

		virtual void onLoadShaders(const IRenderContext* context) override
		{
			D(d);
			IRenderContextProxy proxy(d->L);
			proxy.set(context);
			lua_rawgeti(d->L, LUA_REGISTRYINDEX, d->onLoadShaders);
			d->lua->protectedCall(nullptr, { proxy } );
		}
	};
}

//////////////////////////////////////////////////////////////////////////
IGameHandlerProxy::IGameHandlerProxy(GMLuaCoreState* L)
	: GMAnyProxy(L)
{
	D(d);
	D_BASE(db, Base);
	GameHandlerImpl* impl = new GameHandlerImpl(L);
	db->__handler = impl;
	d->shaderCallback = impl;
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

//////////////////////////////////////////////////////////////////////////
