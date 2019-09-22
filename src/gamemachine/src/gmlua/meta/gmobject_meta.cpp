#include "stdafx.h"
#include "gmobject_meta.h"
#include <gmlua.h>
#include <gmobject.h>

#define NAME "GMObject"

BEGIN_NS

namespace luaapi
{
	GMFunctionReturn gmlua_gc(GMLuaCoreState* L)
	{
		// 如果一个对象是自动释放的，当__gc被调用时，它将提前释放
		GMLuaArguments args(L, NAME ".__gc", { GMMetaMemberType::Object } );
		GMObjectProxy self(L);
		args.getHandler(0, &self);
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

#define GM_LUA_PROXY_META GM_META_WITH_TYPE(__handler, GMMetaMemberType::Pointer); GM_META(__name);

	GMObjectProxy::GMObjectProxy(GMLuaCoreState* l, GMObject* handler /*= nullptr*/)
	{
		GM_CREATE_DATA();
		D(d);
		d->l = l;
		set(handler);

		// 通过构造传入的对象，默认管理其生命周期。
		if (handler)
			setAutoRelease(true);
	}

	GMAnyProxy::~GMAnyProxy()
	{

	}

	bool GMObjectProxy::registerMeta()
	{
		D(d);
		GM_LUA_PROXY_META;
		GM_META_FUNCTION(__gc);
		GM_META_FUNCTION(connect);
		GM_META_FUNCTION(emitSignal);
		setObjectName(getObjectName());
		return true;
	}

	const GMString& GMObjectProxy::getObjectName()
	{
		D(d);
		return d->__name;
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

	GMLuaCoreState* GMObjectProxy::getLuaCoreState() const
	{
		D(d);
		return d->l;
	}

	void GMObjectProxy::setObjectName(const GMString& name)
	{
		D(d);
		d->__name = name;
	}

	GMObject* GMObjectProxy::get() const
	{
		D(d);
		return d->__handler;
	}

	GMObject* GMObjectProxy::operator->() const
	{
		return get();
	}

	void GMObjectProxy::set(GMObject* handler)
	{
		D(d);
		d->__handler = handler;
	}

	GMObjectProxy::~GMObjectProxy()
	{

	}

	void GMAnyProxy::setObjectName(const GMString& name)
	{
		D(d);
		d->__name = name;
	}

	GMLuaCoreState* GMAnyProxy::getLuaCoreState() const
	{
		D(d);
		return d->l;
	}

	IDestroyObject* GMAnyProxy::get() const
	{
		D(d);
		return d->__handler;
	}

	IDestroyObject* GMAnyProxy::operator->() const
	{
		return get();
	}

	void GMAnyProxy::set(IDestroyObject* handler)
	{
		D(d);
		d->__handler = handler;
	}

	/*
	 * __gc([self])
	 */
	GM_LUA_PROXY_IMPL(GMObjectProxy, __gc)
	{
		return gmlua_gc(L);
	}

	/*
	 * connect([self], sender, signal, callback)
	 */
	GM_LUA_PROXY_IMPL(GMObjectProxy, connect)
	{
		GMLuaArguments args(L, NAME ".connect", { GMMetaMemberType::Object, GMMetaMemberType::Object, GMMetaMemberType::String, GMMetaMemberType::Int });
		GMObjectProxy self(L), sender(L);
		args.getArgument(0, &self);
		args.getArgument(1, &sender);
		GMString signal = args.getArgument(2).toString();
		GMLuaReference callback = args.getArgument(3).toInt();
		if (self)
		{
			self->connect(*sender.get(), signal, [L, callback](GMObject* s, GMObject* r) {
				GMObjectProxy sender(L), receiver(L);
				sender.set(s);
				receiver.set(r);
				GMLua l(L);
				l.protectedCall(callback, { sender, receiver });
				l.freeReference(callback);
			});
		}
		return GMReturnValues();
	}

	/*
	 * emit([self], signal)
	 */
	GM_LUA_PROXY_IMPL(GMObjectProxy, emitSignal)
	{
		GMLuaArguments args(L, NAME ".emit", { GMMetaMemberType::Object, GMMetaMemberType::String });
		GMObjectProxy self(L);
		args.getArgument(0, &self);
		GMString signal = args.getArgument(1).toString();
		if (self)
			self->emitSignal(signal);
		return GMReturnValues();
	}
	//////////////////////////////////////////////////////////////////////////
	GMAnyProxy::GMAnyProxy(GMLuaCoreState* l, IDestroyObject* handler /*= nullptr*/)
	{
		GM_CREATE_DATA();
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
		setObjectName(getObjectName());
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

	const GMString& GMAnyProxy::getObjectName()
	{
		D(d);
		return d->__name;
	}

	/*
	 * __gc([self])
	 */
	GM_LUA_PROXY_IMPL(GMAnyProxy, __gc)
	{
		static const GMString s_invoker = NAME ".__gc";
		return gmlua_gc(L);
	}
}

END_NS