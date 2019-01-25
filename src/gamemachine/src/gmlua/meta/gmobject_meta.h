#ifndef __GM_LUA_GMOBJECT_META_H__
#define __GM_LUA_GMOBJECT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

#define LUA_PROXY(className) bool __detached = false; className* __handler = nullptr; GMString __name = #className;

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMObjectProxy)
	{
		LUA_PROXY(GMObject);

		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(connect);
		GMLuaCoreState* l = nullptr;
	};

	class GMObjectProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMObjectProxy)
		GM_ALLOW_COPY(GMObjectProxy)

	public: 
		GMObjectProxy(GMLuaCoreState* l, GMObject* handler = nullptr);

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

	public:
		void setAutoRelease(bool);
		void set(GMObject* handler);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif