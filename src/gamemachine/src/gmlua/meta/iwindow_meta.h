#ifndef __GM_LUA_IWINDOW_META_H__
#define __GM_LUA_IWINDOW_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT_UNALIGNED(IWindowProxy);
	class IWindowProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IWindow, GMAnyProxy)
		GM_DECLARE_PRIVATE(IWindowProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IWindowProxy(GMLuaCoreState* l, IDestroyObject* handler = nullptr);
		~IWindowProxy();

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif