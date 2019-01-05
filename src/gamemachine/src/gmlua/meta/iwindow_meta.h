#ifndef __GM_LUA_IWINDOW_META_H__
#define __GM_LUA_IWINDOW_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IWindowProxy)
	{
		GM_LUA_PROXY(IWindow);
		GM_LUA_PROXY_FUNC(__gc);
		GM_LUA_PROXY_FUNC(create);
		GM_LUA_PROXY_FUNC(centerWindow);
		GM_LUA_PROXY_FUNC(showWindow);
		GM_LUA_PROXY_FUNC(setHandler);
	};

	class IWindowProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(IWindowProxy, IWindow)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif