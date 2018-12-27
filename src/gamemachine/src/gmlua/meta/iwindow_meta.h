#ifndef __GM_LUA_IWINDOW_META_H__
#define __GM_LUA_IWINDOW_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IWindowProxy)
	{
		GM_LUA_PROXY;
		IWindow* window = nullptr;

		GMString __name = "IWindow";
		GM_LUA_META_FUNCTION(__gc);
		GM_LUA_META_FUNCTION(create);
		GM_LUA_META_FUNCTION(centerWindow);
		GM_LUA_META_FUNCTION(showWindow);
		GM_LUA_META_FUNCTION(setHandler);
	};

	class IWindowProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(IWindowProxy)
		GM_LUA_PROXY_FUNCTIONS(IWindow, window)

	public:
		IWindowProxy(IWindow* window = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif