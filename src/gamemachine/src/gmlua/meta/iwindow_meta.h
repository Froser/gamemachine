#ifndef __GM_LUA_IWINDOW_META_H__
#define __GM_LUA_IWINDOW_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMWindowProxy)
	{
		IWindow* window = nullptr;

		GMString __name = "GMWindowProxy";
		GM_LUA_META_FUNCTION(__gc);
		GM_LUA_META_FUNCTION(create);
		GM_LUA_META_FUNCTION(centerWindow);
		GM_LUA_META_FUNCTION(showWindow);
		GM_LUA_META_FUNCTION(setHandler);
	};

	class GMWindowProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMWindowProxy)
		typedef IWindow RealType;

	public:
		GMWindowProxy(IWindow* window = nullptr);

	protected:
		virtual bool registerMeta() override;

	public:
		RealType* get()
		{
			D(d);
			return d->window;
		}

		RealType* operator->()
		{
			return get();
		}

		operator bool() const
		{
			D(d);
			return !!d->window;
		}

		void detach()
		{
			D(d);
			d->window = nullptr;
		}

		void release()
		{
			D(d);
			if (d->window)
			{
				GM_delete(d->window);
				detach();
			}
		}
	};
}

END_NS
#endif