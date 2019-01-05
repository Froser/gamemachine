#ifndef __GM_LUA_IGAMEHANDLERPROXY_META_H__
#define __GM_LUA_IGAMEHANDLERPROXY_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGameHandlerProxy)
	{
		GM_LUA_PROXY(IGameHandler);
		IShaderLoadCallback* shaderCallback = nullptr;
		GMLuaReference init = 0;
		GMLuaReference start = 0;
		GMLuaReference event = 0;
		GMLuaReference onLoadShaders = 0;
		GM_LUA_PROXY_FUNC(__gc);
	};

	class IGameHandlerProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT_NO_DEFAULT_CONSTRUCTOR(IGameHandlerProxy, IGameHandler)

	public:
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