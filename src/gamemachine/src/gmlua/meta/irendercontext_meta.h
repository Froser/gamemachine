#ifndef __GM_LUA_IRENDERCONTEXT_META_H__
#define __GM_LUA_IRENDERCONTEXT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "iwindow_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IRenderContextProxy)
	{
		GM_LUA_PROXY_WITH_TYPE(IRenderContext, const IRenderContext);
		GM_LUA_PROXY_FUNC(getWindow);
		GM_LUA_PROXY_FUNC(getEngine);
	};

	class IRenderContextProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(IRenderContextProxy, const IRenderContext)

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif