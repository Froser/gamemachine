#ifndef __GM_LUA_IGAMEHANDLERPROXY_META_H__
#define __GM_LUA_IGAMEHANDLERPROXY_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_CLASS(IGameHandlerProxy);
	class IGameHandlerProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IGameHandler, GMAnyProxy)
		GM_DECLARE_PRIVATE(IGameHandlerProxy)
		GM_DECLARE_BASE(GMAnyProxy)

	public:
		IGameHandlerProxy() = delete;
		IGameHandlerProxy(GMLuaCoreState* L);
		~IGameHandlerProxy();

	public:
		void init();
		IShaderLoadCallback* getShaderLoadCallback() GM_NOEXCEPT;

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif