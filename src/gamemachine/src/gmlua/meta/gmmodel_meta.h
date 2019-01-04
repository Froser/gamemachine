#ifndef __GM_LUA_GMMODEL_META_H__
#define __GM_LUA_GMMODEL_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMModelProxy)
	{
		GM_LUA_PROXY;
		GMModel* model = nullptr;
		GMString __name = "GMModel";
		GM_LUA_META_FUNCTION(getShader);
	};

	class GMModelProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMModelProxy, GMModel, model)

	public:
		GMModelProxy(GMModel* model = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif