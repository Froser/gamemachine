#ifndef __GM_LUA_IGAMEHANDLERPROXY_META_H__
#define __GM_LUA_IGAMEHANDLERPROXY_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGameHandlerProxy)
	{
		IGameHandler* handler = nullptr;
		IShaderLoadCallback* shaderCallback = nullptr;

		GMLuaReference init = 0;
		GMLuaReference start = 0;
		GMLuaReference event = 0;
		GMLuaReference onLoadShaders = 0;

		GMString __name = L"IGameHandler";
		GM_LUA_META_FUNCTION(__gc);
	};

	class IGameHandlerProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(IGameHandlerProxy)
		GM_LUA_META_PROXY_FUNCTIONS(IGameHandler, handler)

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