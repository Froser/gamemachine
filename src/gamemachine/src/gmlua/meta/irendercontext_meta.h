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
		const IRenderContext* context = nullptr;
		GM_LUA_META_FUNCTION(getWindow);
		GM_LUA_META_FUNCTION(getEngine);
	};

	class IRenderContextProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(IRenderContextProxy)
		GM_LUA_META_PROXY_FUNCTIONS(const IRenderContext, context)

	public:
		IRenderContextProxy(const IRenderContext* context = nullptr);

	public:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif