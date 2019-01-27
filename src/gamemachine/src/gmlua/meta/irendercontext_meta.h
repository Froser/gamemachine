#ifndef __GM_LUA_IRENDERCONTEXT_META_H__
#define __GM_LUA_IRENDERCONTEXT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "iwindow_meta.h"
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IRenderContextProxy)
	{
		GM_LUA_PROXY_FUNC(getWindow);
		GM_LUA_PROXY_FUNC(getEngine);
	};

	class IRenderContextProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(const IRenderContext, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IRenderContextProxy, GMAnyProxy)

	public:
		IRenderContextProxy(const IRenderContextProxy& rhs);

	public:
		void set(const IRenderContext* o);
		void setLuaCoreState(GMLuaCoreState* l);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif