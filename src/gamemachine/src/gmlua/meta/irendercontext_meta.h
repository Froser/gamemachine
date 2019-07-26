#ifndef __GM_LUA_IRENDERCONTEXT_META_H__
#define __GM_LUA_IRENDERCONTEXT_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "iwindow_meta.h"
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(IRenderContextProxy);
	class IRenderContextProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(const IRenderContext, GMAnyProxy)
		GM_DECLARE_PRIVATE(IRenderContextProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IRenderContextProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		IRenderContextProxy(const IRenderContextProxy& rhs);
		~IRenderContextProxy();

	public:
		void set(const IRenderContext* o);
		void setLuaCoreState(GMLuaCoreState* l);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif