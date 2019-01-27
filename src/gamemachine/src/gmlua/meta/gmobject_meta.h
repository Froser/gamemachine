#ifndef __GM_LUA_GMOBJECT_META_H__
#define __GM_LUA_GMOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

#define LUA_PROXY(className) className* __handler = nullptr; GMString __name = #className;

namespace luaapi
{
	GMFunctionReturn gmlua_gc(GMLuaCoreState*);

	GM_PRIVATE_OBJECT(GMObjectProxy)
	{
		LUA_PROXY(GMObject);

		GMFunctionReturn (*__gc)(GMLuaCoreState*) = gmlua_gc;
		GM_LUA_PROXY_FUNC(connect);
		GMLuaCoreState* l = nullptr;
	};

	class GMObjectProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMObjectProxy)

	public: 
		GMObjectProxy(GMLuaCoreState* l, GMObject* handler = nullptr);

		GMLuaCoreState* getLuaCoreState() const
		{
			D(d);
			return d->l;
		}

		void setObjectName(const GMString& name)
		{
			D(d);
			d->__name = name;
		}

		GMObject* get() const
		{
			D(d);
			return d->__handler;
		}

		GMObject* operator->() const
		{
			return get();
		}

		operator bool() const
		{
			D(d);
			return !!d->__handler;
		}

		void set(GMObject* handler)
		{
			D(d);
			d->__handler = handler;
		}

	public:
		void setAutoRelease(bool);

	protected:
		virtual bool registerMeta() override;
	};

	//////////////////////////////////////////////////////////////////////////
	GM_PRIVATE_OBJECT(GMAnyProxy)
	{
		LUA_PROXY(IDestroyObject);

		GMFunctionReturn(*__gc)(GMLuaCoreState*) = gmlua_gc;
		GM_LUA_PROXY_FUNC(connect);
		GMLuaCoreState* l = nullptr;
	};

	class GMAnyProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMAnyProxy)

	public:
		GMAnyProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);

		void setObjectName(const GMString& name)
		{
			D(d);
			d->__name = name;
		}

		GMLuaCoreState* getLuaCoreState() const
		{
			D(d);
			return d->l;
		}

		IDestroyObject* get() const
		{
			D(d);
			return d->__handler;
		}

		IDestroyObject* operator->() const
		{
			return get();
		}

		operator bool() const
		{
			D(d);
			return !!d->__handler;
		}

		void set(IDestroyObject* handler)
		{
			D(d);
			d->__handler = handler;
		}

	public:
		void setAutoRelease(bool);

	protected:
		virtual bool registerMeta() override;
	};
}

#undef LUA_PROXY

END_NS
#endif