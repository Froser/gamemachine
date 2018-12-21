#ifndef __GM_LUA_IWINDOW_META_H__
#define __GM_LUA_IWINDOW_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGameHandlerProxy)
	{
		IGameHandler* handler = nullptr;

		GMString __name = "IGameHandlerProxy";
		GM_LUA_META_FUNCTION(__gc);
	};

	class IGameHandlerProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(IGameHandlerProxy)
		typedef IGameHandler RealType;

	public:
		IGameHandlerProxy(IGameHandler* handler = nullptr);

	protected:
		virtual bool registerMeta() override;

	public:
		RealType* get()
		{
			D(d);
			return d->handler;
		}

		RealType* operator->()
		{
			return get();
		}

		operator bool() const
		{
			D(d);
			return !!d->handler;
		}

		void detach()
		{
			D(d);
			d->handler = nullptr;
		}

		void release()
		{
			D(d);
			if (d->handler)
			{
				GM_delete(d->handler);
				detach();
			}
		}
	};
}

END_NS
#endif