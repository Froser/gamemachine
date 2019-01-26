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
		GM_ALLOW_COPY(GMObjectProxy)

	public: 
		GMObjectProxy(GMLuaCoreState* l, GMObject* handler = nullptr);

		void setObjectName(const GMString& name)
		{
			D(d);
			d->__name = name;
		}

		void copyData(const GMObject& a)
		{
			D(d);
			const GMObjectProxy& i = static_cast<const GMObjectProxy&>(a);
			D_OF(d_other, &i);
			d->__handler = d_other->__handler;
		}

		GMObject* get()
		{
			D(d);
			return d->__handler;
		}

		GMObject* operator->()
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
		GM_ALLOW_COPY(GMAnyProxy)

	public:
		GMAnyProxy(GMLuaCoreState* l, GMObject* handler = nullptr);

		void setObjectName(const GMString& name)
		{
			D(d);
			d->__name = name;
		}

		void copyData(const GMObject& a)
		{
			D(d);
			const GMAnyProxy& i = static_cast<const GMAnyProxy&>(a);
			D_OF(d_other, &i);
			d->__handler = d_other->__handler;
		}

		IDestroyObject* get()
		{
			D(d);
			return d->__handler;
		}

		IDestroyObject* operator->()
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
}

#undef LUA_PROXY

END_NS
#endif