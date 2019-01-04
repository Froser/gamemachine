#ifndef __GM_LUA_GMSHADER_META_H__
#define __GM_LUA_GMSHADER_META_H__
#include <gmcommon.h>
#include <gmlua.h>
#include <gmshader.h>
BEGIN_NS

namespace luaapi
{
	GM_PRIVATE_OBJECT(GMShaderProxy)
	{
		GM_LUA_PROXY;
		GMShader* shader = nullptr;
		GMString __name = "GMShader";
		GM_LUA_PROXY_FUNC(__index);
	};

	class GMShaderProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMShaderProxy, GMShader, shader)

	public:
		GMShaderProxy(GMShader* shader = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

	// 以下是GMShader各种属性
	GM_PRIVATE_OBJECT(GMMaterialProxy)
	{
		GM_LUA_PROXY;
		GMMaterial* material = nullptr;
		GMString __name = "GMMaterial";
		GM_LUA_PROXY_FUNC(__index);
		GM_LUA_PROXY_FUNC(__newindex);
	};

	class GMMaterialProxy : public GMObject
	{
		GM_LUA_PROXY_OBJECT(GMMaterialProxy, GMMaterial, material)

	public:
		GMMaterialProxy(GMMaterial* material = nullptr);

	protected:
		virtual bool registerMeta() override;
	};

}

END_NS
#endif