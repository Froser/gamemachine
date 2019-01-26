#ifndef __GM_LUA_IGAMEHANDLERPROXY_META_H__
#define __GM_LUA_IGAMEHANDLERPROXY_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include "gmobject_meta.h"
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGameHandlerProxy)
	{
		IShaderLoadCallback* shaderCallback = nullptr;
		GMLuaReference init = 0;
		GMLuaReference start = 0;
		GMLuaReference event = 0;
		GMLuaReference onLoadShaders = 0;
	};

	class IGameHandlerProxy : public GMAnyProxy
	{
		GM_LUA_PROXY_OBJ(IGameHandler, GMAnyProxy)
		GM_DECLARE_PRIVATE_AND_BASE(IGameHandlerProxy, GMAnyProxy)

	public:
		IGameHandlerProxy() = delete;
		IGameHandlerProxy(GMLuaCoreState* L);

	public:
		void init();
		IShaderLoadCallback* getShaderLoadCallback() GM_NOEXCEPT;

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif