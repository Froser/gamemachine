#ifndef __GM_LUA_IGRAPHICENGINE_META_H__
#define __GM_LUA_IGRAPHICENGINE_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(IGraphicEngineProxy)
	{
		GM_LUA_PROXY;
		IGraphicEngine* engine = nullptr;
	};

	class IGraphicEngineProxy : public GMObject
	{
		GM_DECLARE_PRIVATE(IGraphicEngineProxy)
		GM_LUA_META_PROXY_FUNCTIONS(IGraphicEngine, engine)

	public:
		IGraphicEngineProxy(IGraphicEngine* engine = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif