#ifndef __GM_LUA_IWINDOW_META_H__
#define __GM_LUA_IWINDOW_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IWindowProxy)
	{
		GM_LUA_PROXY_FUNC(create);
		GM_LUA_PROXY_FUNC(centerWindow);
		GM_LUA_PROXY_FUNC(showWindow);
		GM_LUA_PROXY_FUNC(setHandler);
		GM_LUA_PROXY_FUNC(getInputManager);
		GM_LUA_PROXY_FUNC(addWidget);
	};

	class IWindowProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IWindow, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IWindowProxy, GMAnyProxy)

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif