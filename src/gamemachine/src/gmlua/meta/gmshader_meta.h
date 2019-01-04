#ifndef __GM_LUA_GMSHADER_META_H__
#define __GM_LUA_GMSHADER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMShaderProxy)
	{
		GM_LUA_PROXY;
		GMShader* shader = nullptr;
		GMString __name = "GMShader";
		GM_LUA_META_FUNCTION(__index);
		GM_LUA_META_FUNCTION(__newindex);
	};

	class GMShaderProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMShaderProxy, GMShader, shader)

	public:
		GMShaderProxy(GMShader* shader = nullptr);

	protected:
		virtual bool registerMeta() override;
	};
}

END_NS
#endif