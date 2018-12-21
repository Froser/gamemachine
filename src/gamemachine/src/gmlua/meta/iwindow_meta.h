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
	};

	class GMWindowProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(GMWindowProxy)

	public:
		GMWindowProxy(IWindow* window = nullptr);

	protected:
		virtual bool registerMeta() override;

	public:
		IWindow* get()
		{
			D(d);
			return d->window;
		}

		IWindow* operator->()
		{
			return get();
		}

		operator bool() const
		{
			D(d);
			return !!d->window;
		}

	};
}

END_NS
#endif