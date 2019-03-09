#include "stdafx.h"
#include "gmobject_meta.h"
#include <gmlua.h>
#include <gmobject.h>

#define NAME "GMObject"

namespace gm
{
	namespace luaapi
	{
		GMFunctionReturn gmlua_gc(GMLuaCoreState* L)
		{
			// 如果一个对象是自动释放的，当__gc被调用时，它将提前释放
			static const GMString s_invoker = NAME ".__gc";
			GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
			GMObjectProxy self(L);
			GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
			if (self)
			{
				GMLuaRuntime* rt = GMLua::getRuntime(L);
				if (rt->containsObject(self.get()))
				{
					bool b = rt->detachObject(self.get());
					GM_ASSERT(b);
					GM_delete(self.get());
				}
			}
			return GMReturnValues();
		}
	}
}

using namespace luaapi;

#define GM_LUA_PROXY_META GM_META_WITH_TYPE(__handler, GMMetaMemberType::Pointer); GM_META(__name);

GMObjectProxy::GMObjectProxy(GMLuaCoreState* l, GMObject* handler /*= nullptr*/)
{
	D(d);
	d->l = l;
	set(handler);

	// 通过构造传入的对象，默认管理其生命周期。
	if (handler)
		setAutoRelease(true);
}

bool GMObjectProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	GM_META_FUNCTION(connect);
	GM_META_FUNCTION(emitSignal);
	return true;
}

void GMObjectProxy::setAutoRelease(bool autorelease)
{
	D(d);
	if (autorelease)
	{
		if (d->__handler)
		{
			// 这个对象放入runtime托管池
			GMLua::getRuntime(d->l)->addObject(d->__handler);
		}
	}
	else
	{
		// 从托管池移除，自己管理生命周期。
		GMLua::getRuntime(d->l)->detachObject(d->__handler);
	}
}

/*
 * __gc([self])
 */
GM_LUA_PROXY_IMPL(GMObjectProxy, __gc)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	return gmlua_gc(L);
}

/*
 * connect([self], sender, signal, callback)
 */
GM_LUA_PROXY_IMPL(GMObjectProxy, connect)
{
	static const GMString s_invoker = NAME ".connect";
	GM_LUA_CHECK_ARG_COUNT(L, 4, NAME ".connect");
	GMObjectProxy self(L), sender(L);
	GMLuaReference callback = GMArgumentHelper::popArgument(L, s_invoker).toInt(); // callback
	GMString signal = GMArgumentHelper::popArgumentAsString(L, s_invoker); //signal
	GMArgumentHelper::popArgumentAsObject(L, sender, s_invoker); //sender
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
	{
		self->connect(*sender.get(), signal, [L, callback](GMObject* s, GMObject* r) {
			GMObjectProxy sender(L), receiver(L);
			sender.set(s);
			receiver.set(r);
			GMLua l(L);
			l.protectedCall(callback, { sender, receiver });
		});
	}
	return GMReturnValues();
}

/*
 * emit([self], signal)
 */
GM_LUA_PROXY_IMPL(GMObjectProxy, emitSignal)
{
	static const GMString s_invoker = NAME ".emit";
	GM_LUA_CHECK_ARG_COUNT(L, 2, NAME ".emit");
	GMObjectProxy self(L), sender(L);
	GMString signal = GMArgumentHelper::popArgumentAsString(L, s_invoker); //signal
	GMArgumentHelper::popArgumentAsObject(L, self, s_invoker); //self
	if (self)
		self->emitSignal(signal);
	return GMReturnValues();
}
//////////////////////////////////////////////////////////////////////////
GMAnyProxy::GMAnyProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
{
	D(d);
	d->l = l;
	set(handler);

	// 通过构造传入的对象，默认管理其生命周期。
	if (handler)
		setAutoRelease(true);
}

bool GMAnyProxy::registerMeta()
{
	D(d);
	GM_LUA_PROXY_META;
	GM_META_FUNCTION(__gc);
	return true;
}

void GMAnyProxy::setAutoRelease(bool autorelease)
{
	D(d);
	if (autorelease)
	{
		if (d->__handler)
		{
			// 这个对象放入runtime托管池
			GMLua::getRuntime(d->l)->addObject(d->__handler);
		}
	}
	else
	{
		// 从托管池移除，自己管理生命周期。
		GMLua::getRuntime(d->l)->detachObject(d->__handler);
	}
}

/*
 * __gc([self])
 */
GM_LUA_PROXY_IMPL(GMAnyProxy, __gc)
{
	static const GMString s_invoker = NAME ".__gc";
	GM_LUA_CHECK_ARG_COUNT(L, 1, NAME ".__gc");
	return gmlua_gc(L);
}
